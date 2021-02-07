// SPDX-License-Identifier: GPL-2.0
/*
 * Implement the manual drop-all-pagecache function
 */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/writeback.h>
#include <linux/sysctl.h>
#include <linux/gfp.h>
#if defined(CONFIG_POWERSUSPEND)
#include <linux/powersuspend.h>
#endif
#include <linux/delay.h>
#include "internal.h"

/* A global variable is a bit ugly, but it keeps the code simple */
int sysctl_drop_caches;

static void drop_pagecache_sb(struct super_block *sb, void *unused)
{
	struct inode *inode, *toput_inode = NULL;

	spin_lock(&sb->s_inode_list_lock);
	list_for_each_entry(inode, &sb->s_inodes, i_sb_list) {
		spin_lock(&inode->i_lock);
		/*
		 * We must skip inodes in unusual state. We may also skip
		 * inodes without pages but we deliberately won't in case
		 * we need to reschedule to avoid softlockups.
		 */
		if ((inode->i_state & (I_FREEING|I_WILL_FREE|I_NEW)) ||
		    (inode->i_mapping->nrpages == 0 && !need_resched())) {
			spin_unlock(&inode->i_lock);
			continue;
		}
		__iget(inode);
		spin_unlock(&inode->i_lock);
		spin_unlock(&sb->s_inode_list_lock);

		invalidate_mapping_pages(inode->i_mapping, 0, -1);
		iput(toput_inode);
		toput_inode = inode;

		cond_resched();
		spin_lock(&sb->s_inode_list_lock);
	}
	spin_unlock(&sb->s_inode_list_lock);
	iput(toput_inode);
}

void mm_drop_caches(int val)
{
	if (val & 1) {
		iterate_supers(drop_pagecache_sb, NULL);
		count_vm_event(DROP_PAGECACHE);
	}
	if (val & 2) {
		drop_slab();
		count_vm_event(DROP_SLAB);
	}
}

int drop_caches_sysctl_handler(struct ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (ret)
		return ret;
	if (write) {
		static int stfu;

		mm_drop_caches(sysctl_drop_caches);

		if (!stfu) {
			pr_info("%s (%d): drop_caches: %d\n",
				current->comm, task_pid_nr(current),
				sysctl_drop_caches);
		}
		stfu |= sysctl_drop_caches & 4;
	}
	return 0;
}
#ifdef CONFIG_POWERSUSPEND
static void drop_caches_suspend(struct work_struct *work);
static DECLARE_WORK(drop_caches_suspend_work, drop_caches_suspend);

static void drop_caches_suspend(struct work_struct *work)
{
	/* sleep for 200ms */
	msleep(200);
	/* sync */
	emergency_sync();
	/* echo "3" > /proc/sys/vm/drop_caches */
	iterate_supers(drop_pagecache_sb, NULL);
        drop_slab();
}

static void __ref drop_caches_power_suspend(struct power_suspend *handler)
{
			schedule_work_on(0, &drop_caches_suspend_work);
}

static struct power_suspend drop_caches_power_suspend_driver = {
	.suspend = drop_caches_power_suspend,
};

static int __init drop_caches_init(void)
{
	register_power_suspend(&drop_caches_power_suspend_driver);
	return 0;
}
late_initcall(drop_caches_init);
#endif
