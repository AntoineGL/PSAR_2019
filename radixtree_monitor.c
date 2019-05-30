#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/radix-tree.h>
#include <linux/dcache.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/fdtable.h>
#include <linux/fs_struct.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/debugfs.h> 
#include <linux/uaccess.h>
#include "utils.h"
MODULE_DESCRIPTION("Radix tree state show !");
MODULE_AUTHOR("Maria");
MODULE_LICENSE("GPL");

char fname[PAGE_SIZE]; 
#define SIZE (PAGE_SIZE*20)

static void my_dump_node(struct radix_tree_node *node, unsigned long index,
		struct seq_file *s, size_t *cpt, struct radix_tree_root *root)
{
	unsigned long i;

	/*info about current node*/
	//*cpt += scnprintf(buf, SIZE - *cpt,
	seq_printf(s, "\n#####_ Radix Node: %p offset %d indices %lu-%lu\n\
			\tparent %p tags %lx %lx %lx\n\
			\tshift %d count %d exceptional %d \n\n",
			node, node->offset, index, index | node_maxindex(node),
			node->parent,
			tag_get(node, 0, node->offset), 
			tag_get(node, 1, node->offset),
			tag_get(node, 2, node->offset),
			//node->tags[0][0], node->tags[1][0], node->tags[2][0],
			node->shift, node->count, node->exceptional);

	/*check every slot from array*/
	for (i = 0; i < RADIX_TREE_MAP_SIZE; i++) {
		unsigned long first = index | (i << node->shift);
		unsigned long last = first | ((1UL << node->shift) - 1);
		void *entry = node->slots[i];

		/*case 1 : no entry for this index*/
		if (!entry) {
			//pr_info("slot[%ld] : NO ENTRY in the tree for this index.\n", i);
			continue;
		}

		/*case 2 : the entry in this slot has been moved
 		to another location in the tree and the lookup should be restarted*/
		if (entry == RADIX_TREE_RETRY) {
			pr_info("slot[%ld] :  RETRY entry, indices %lu-%lu, parent %p\n",
					i, first, last, node);

		/*case 3 : not internal node (exceptional or data)*/
		} else if (!radix_tree_is_internal_node(entry)) {
			if(radix_tree_is_data_node(entry)){
				//*cpt += scnprintf(buf, SIZE - *cpt,
				seq_printf(s, 
					"slot[%ld] : DATA entry %p indices %lu-%lu\n\t\tparent %p\
					\n\t\ttags: DIRTY %d WRITEBACK %d TOWRITE %d\n",
					i, entry, first, last, node,
					 radix_tree_tag_get(root, first, 0),
					 radix_tree_tag_get(root, first, 1),
					 radix_tree_tag_get(root, first, 2));
			}		
		/*case 4 : sibling slot (points directly to another slot in the same node)*/
		} else if (is_sibling_entry(node, entry)) {
			pr_info("slot[%ld] : SIBLING %p, indices %lu-%lu, parent %p, val %p\n",
					i, entry, first, last, node,
					*(void **)entry_to_node(entry));

		/*case 5 : next level*/
		} else {
			//pr_info("radix slot[%ld] : NEXT LEVEL\n", i);
			my_dump_node(entry_to_node(entry), first, s, cpt, root);
		}
	}
}

static int dfile_show(struct seq_file *s, void *ignored)
{
	ssize_t cpt = 0;
	int error;
	struct radix_tree_root *root;
	char buf[SIZE];
	/*get filnename to monitor*/
	struct path path_f; 
	error = kern_path(fname, LOOKUP_FOLLOW, &path_f); 
	if (error)
		return ERR_PTR(error);
	struct inode *inode_f ;

	if (!path_f.dentry){
		pr_info("no dentry\n");
		return 0;
	} else {
		inode_f = d_backing_inode(path_f.dentry);// backing or via struct ?
	}
	root = &inode_f->i_mapping->i_pages;// i_data or i_mapping ?
	
	/*radix-tree root*/
	pr_info("Radix-tree root : start.\n");
	//cpt += scnprintf(buf, SIZE - cpt,
	seq_printf(s, 
			"\n\nRadix Tree Root: %p rnode %p\n\t\ttags: DIRTY %d WRITEBACK %d TOWRITE %d\n\n",
			root, root->rnode,
			root_tag_get(root, 0),
			root_tag_get(root, 1), 
			root_tag_get(root, 2));

	if (!radix_tree_is_internal_node(root->rnode)){
		if(radix_tree_is_data_node(root->rnode)){
			/*root is data ptr*/
			pr_info("Radix-tree root is data node.\n");
		}	
	}
	else /*go to node*/
		my_dump_node(entry_to_node(root->rnode), 0, s, &cpt, root);
	/*exit*/
	pr_info("Radix-tree : finish.\n");
	return 0;
}

static ssize_t __used radix_tree_store(struct kobject *kobj,
				  struct kobj_attribute *attr,
				  const char *buf, size_t count)
{
	scnprintf(fname, PAGE_SIZE-1, buf);
	pr_info("Users filename to monitor %s.\n", fname);
	return count;
}

/* definir un attribut */
static struct kobj_attribute radix_tree_attr = __ATTR_WO(radix_tree);

struct dentry *dfile;
void *data;

static int dfile_open(struct inode *inode, struct file *file)
{
	return single_open(file, dfile_show, inode->i_private);
}
static const struct file_operations fops = { 
	.open		= dfile_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release, 
}; 

static int __init radixtree_init(void)
{
	int error = 0;
	pr_info("Module radix_tree initialized.\n");
	snprintf(fname, PAGE_SIZE - 1, "file.txt"); //default filename
	pr_info("default filename to monitor %s\n", fname);
	
	dfile = kmalloc(sizeof(*dfile), GFP_KERNEL);
	dfile = debugfs_create_file("radixtree", 0660, 0, data, &fops);

	error = sysfs_create_file(kernel_kobj, &radix_tree_attr.attr);
    	if (error) {
		pr_debug("failed to create the radix_tree file in /sys/kernel\n");
   	}
	return error;
}

static void __exit radixtree_exit(void)
{
    	sysfs_remove_file(kernel_kobj, &radix_tree_attr.attr);
	debugfs_remove(dfile);
	pr_info("Module radix_tree uninitialized.\n");
}

module_init(radixtree_init);
module_exit(radixtree_exit);
