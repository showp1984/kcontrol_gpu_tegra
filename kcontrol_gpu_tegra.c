/*
 * KControl GPU module for tegra devices
 *
 * Copyright (c) 2013 Dennis Rassmann
 * Author: Dennis Rassmann <showp1984@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#define REL_KERNELPATH "../3.4.0-modbuild-kernel/"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kallsyms.h>
#include <linux/sysfs.h>
#include "mach-tegra/dvfs.h"

#define THIS_EXPERIMENTAL 1

#define DRIVER_AUTHOR "Dennis Rassmann <showp1984@gmail.com>"
#define DRIVER_DESCRIPTION "KControl GPU module for tegra devices"
#define DRIVER_VERSION "1.0"
#define LOGTAG "kcontrol_gpu_tegra: "

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");


struct global_attr {
	struct attribute attr;
	ssize_t (*show)(struct kobject *kobj,
			struct attribute *attr, char *buf);
	ssize_t (*store)(struct kobject *a, struct attribute *b,
			 const char *c, size_t count);
};

#define define_one_global_ro(_name)		\
static struct global_attr _name =		\
__ATTR(_name, 0444, show_##_name, NULL)

#define define_one_global_rw(_name)		\
static struct global_attr _name =		\
__ATTR(_name, 0644, show_##_name, store_##_name)

/* module parameters */
static uint dvfs_core_table = 0x00000000;
module_param(dvfs_core_table, uint, 0444);

static uint soc_speedo_id = 0x00000000;
module_param(soc_speedo_id, uint, 0444);

static struct dvfs *core_table;
static int *soc_speedo;

struct kobject *kcontrol_gpu_tegra_kobject;

static ssize_t show_tegra_freqs(struct kobject *a, struct attribute *b,
				   char *buf)
{
	ssize_t len = 0;
	int i = 0, j=0;
	if ((core_table != NULL) && (soc_speedo != NULL)) {
		for (i=0; (strcmp(core_table->clk_name, "spdif_out") != 0); i++) {
			if (core_table->speedo_id != soc_speedo)
				continue;
			if ((strcmp(core_table->clk_name, "vde") == 0) ||
				(strcmp(core_table->clk_name, "mpe") == 0) ||
				(strcmp(core_table->clk_name, "2d") == 0) ||
				(strcmp(core_table->clk_name, "epp") == 0) ||
				(strcmp(core_table->clk_name, "3d") == 0) ||
				(strcmp(core_table->clk_name, "3d2") == 0) ||
				(strcmp(core_table->clk_name, "se") == 0) ||
				(strcmp(core_table->clk_name, "cbus") == 0)) {
				for (j=0; core_table->freqs[j] != 0; j++) {
					len += sprintf(buf + len, "%s %u\n", core_table->clk_name, core_table->freqs[i]);
				}
			}
		}
	} else {
		len += sprintf(buf + len, "Error! Pointer == null!\n");
	}
	return len;
}
static ssize_t store_tegra_freqs(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	int i = 0;
	unsigned int pwrlvl = 0;
	long unsigned int hz = 0;
	const char *chz = NULL;

	if ((core_table != NULL) && (soc_speedo != NULL)) {
		for (i=0; i<count; i++) {
			if (buf[i] == ' ') {
				sscanf(&buf[(i-1)], "%u", &pwrlvl);
				chz = &buf[(i+1)];
			}
		}
		sscanf(chz, "%lu", &hz);
		kpwr->pwrlevels[pwrlvl].gpu_freq = hz;
	} else {
		pr_err(LOGTAG"Error! Pointer == null!\n");
	}
	return count;
}
define_one_global_rw(tegra_freqs);

static ssize_t show_version(struct kobject *a, struct attribute *b,
				   char *buf)
{
	ssize_t len = 0;
	len += sprintf(buf + len, DRIVER_VERSION);
	len += sprintf(buf + len, "\n");
	return len;
}
define_one_global_ro(version);

static struct attribute *kcontrol_gpu_tegra_attributes[] = {
	&version.attr,
	&tegra_freqs.attr,
	NULL
};

static struct attribute_group kcontrol_gpu_tegra_attr_group = {
	.attrs = kcontrol_gpu_tegra_attributes,
	.name = "kcontrol_gpu_tegra",
};

static int __init kcontrol_gpu_tegra_init(void)
{
	int rc = 0;

#if THIS_EXPERIMENTAL
    printk(KERN_WARNING LOGTAG "#######################################");
    printk(KERN_WARNING LOGTAG "WARNING: THIS MODULE IS EXPERIMENTAL!\n");
    printk(KERN_WARNING LOGTAG "You have been warned.\n");
	printk(KERN_INFO LOGTAG "%s, version %s\n", DRIVER_DESCRIPTION,	DRIVER_VERSION);
	printk(KERN_INFO LOGTAG "author: %s\n", DRIVER_AUTHOR);
    printk(KERN_WARNING LOGTAG "#######################################");
#else
	printk(KERN_INFO LOGTAG "%s, version %s\n", DRIVER_DESCRIPTION,	DRIVER_VERSION);
	printk(KERN_INFO LOGTAG "author: %s\n", DRIVER_AUTHOR);
#endif

	WARN(dvfs_core_table == 0x00000000, LOGTAG "dvfs_core_table == 0x00000000!");
	WARN(soc_speedo_id == 0x00000000, LOGTAG "soc_speedo_id == 0x00000000!");

	if ((dvfs_core_table != 0x00000000) && (soc_speedo_id != 0x00000000)) {
		core_table = (struct dvfs *)dvfs_core_table;
		soc_speedo = (int *)soc_speedo_id;

		if (kernel_kobj) {
			rc = sysfs_create_group(kernel_kobj, &kcontrol_gpu_tegra_attr_group);
			if (rc) {
				pr_warn(LOGTAG"sysfs: ERROR, could not create sysfs group");
			}
		} else
			pr_warn(LOGTAG"sysfs: ERROR, could not find sysfs kobj");

		pr_info(LOGTAG "everything done, have fun!\n");
	} else {
		pr_err(LOGTAG "Error, you need to insert this module WITH parameters!\n");
		pr_err(LOGTAG "Nothing modified, removing myself!\n");
		return -EAGAIN;
	}
	return 0;
}

static void __exit kcontrol_gpu_tegra_exit(void)
{
	sysfs_remove_group(kernel_kobj, &kcontrol_gpu_tegra_attr_group);
	printk(KERN_INFO LOGTAG "unloaded\n");
}

module_init(kcontrol_gpu_tegra_init);
module_exit(kcontrol_gpu_tegra_exit);

