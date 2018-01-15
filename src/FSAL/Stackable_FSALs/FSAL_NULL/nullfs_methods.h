/* NULLFS methods for handles
 */

#include "../../../include/fsal.h"

struct nullfs_fsal_obj_handle;

struct next_ops {
    struct export_ops exp_ops;              /*< Vector of operations */
    struct fsal_obj_ops obj_ops;            /*< Shared handle methods vector */
    struct fsal_dsh_ops dsh_ops;            /*< Shared handle methods vector */
    const struct fsal_up_vector *up_ops;    /*< Upcall operations */
};

/**
 * Structure used to store data for read_dirents callback.
 *
 * Before executing the upper level callback (it might be another
 * stackable fsal or the inode cache), the context has to be restored.
 */
struct nullfs_readdir_state 
{
    fsal_readdir_cb cb;             /*< Callback to the upper layer. */
    struct nullfs_fsal_export *exp; /*< Export of the current nullfsal. */
    void *dir_state;                /*< State to be sent to the next callback. */
};


extern struct next_ops next_ops;
extern struct fsal_up_vector fsal_up_top;
void nullfs_handle_ops_init(struct fsal_obj_ops *ops);

/*
 * NULLFS internal export
 */
struct nullfs_fsal_export {
    struct fsal_export export;
    /* Other private export data goes here */
};

fsal_status_t nullfs_lookup_path(struct fsal_export *exp_hdl,
                 const char *path,
                 struct fsal_obj_handle **handle,
                 struct attrlist *attrs_out);

fsal_status_t nullfs_create_handle(struct fsal_export *exp_hdl,
                   struct gsh_buffdesc *hdl_desc,
                   struct fsal_obj_handle **handle,
                   struct attrlist *attrs_out);

fsal_status_t nullfs_alloc_and_check_handle(struct nullfs_fsal_export *export,
        struct fsal_obj_handle *sub_handle,
        struct fsal_filesystem *fs,
        struct fsal_obj_handle **new_handle,
        fsal_status_t subfsal_status);

/*
 * NULLFS internal object handle
 *
 * It contains a pointer to the fsal_obj_handle used by the subfsal.
 *
 * AF_UNIX sockets are strange ducks.  I personally cannot see why they
 * are here except for the ability of a client to see such an animal with
 * an 'ls' or get rid of one with an 'rm'.  You can't open them in the
 * usual file way so open_by_handle_at leads to a deadend.  To work around
 * this, we save the args that were used to mknod or lookup the socket.
 */

struct nullfs_fsal_obj_handle {
    struct fsal_obj_handle obj_handle; /*< Handle containing nullfs data.*/
    struct fsal_obj_handle *sub_handle; /*< Handle of the sub fsal.*/
    int32_t refcnt;		/*< Reference count.  This is signed to make
                   mistakes easy to see. */
};

int nullfs_fsal_open(struct nullfs_fsal_obj_handle *, int, fsal_errors_t *);
int nullfs_fsal_readlink(struct nullfs_fsal_obj_handle *, fsal_errors_t *);

static inline bool nullfs_unopenable_type(object_file_type_t type)
{
    if ((type == SOCKET_FILE) || (type == CHARACTER_FILE)
        || (type == BLOCK_FILE)) {
        return true;
    } else {
        return false;
    }
}

    /* I/O management */
fsal_status_t nullfs_open(struct fsal_obj_handle *obj_hdl,
              fsal_openflags_t openflags);
fsal_openflags_t nullfs_status(struct fsal_obj_handle *obj_hdl);
fsal_status_t nullfs_read(struct fsal_obj_handle *obj_hdl,
              uint64_t offset,
              size_t buffer_size, void *buffer,
              size_t *read_amount, bool *end_of_file);
fsal_status_t nullfs_write(struct fsal_obj_handle *obj_hdl,
               uint64_t offset,
               size_t buffer_size, void *buffer,
               size_t *write_amount, bool *fsal_stable);
fsal_status_t nullfs_commit(struct fsal_obj_handle *obj_hdl,	/* sync */
                off_t offset, size_t len);
fsal_status_t nullfs_lock_op(struct fsal_obj_handle *obj_hdl,
                 void *p_owner,
                 fsal_lock_op_t lock_op,
                 fsal_lock_param_t *request_lock,
                 fsal_lock_param_t *conflicting_lock);
fsal_status_t nullfs_share_op(struct fsal_obj_handle *obj_hdl, void *p_owner,
                  fsal_share_param_t request_share);
fsal_status_t nullfs_close(struct fsal_obj_handle *obj_hdl);

/* Multi-FD */
fsal_status_t nullfs_open2(struct fsal_obj_handle *obj_hdl,
               struct state_t *state,
               fsal_openflags_t openflags,
               enum fsal_create_mode createmode,
               const char *name,
               struct attrlist *attrs_in,
               fsal_verifier_t verifier,
               struct fsal_obj_handle **new_obj,
               struct attrlist *attrs_out,
               bool *caller_perm_check);
bool nullfs_check_verifier(struct fsal_obj_handle *obj_hdl,
               fsal_verifier_t verifier);
fsal_openflags_t nullfs_status2(struct fsal_obj_handle *obj_hdl,
                struct state_t *state);
fsal_status_t nullfs_reopen2(struct fsal_obj_handle *obj_hdl,
                 struct state_t *state,
                 fsal_openflags_t openflags);
fsal_status_t nullfs_read2(struct fsal_obj_handle *obj_hdl,
               bool bypass,
               struct state_t *state,
               uint64_t offset,
               size_t buf_size,
               void *buffer,
               size_t *read_amount,
               bool *eof,
               struct io_info *info);
fsal_status_t nullfs_write2(struct fsal_obj_handle *obj_hdl,
                bool bypass,
                struct state_t *state,
                uint64_t offset,
                size_t buf_size,
                void *buffer,
                size_t *write_amount,
                bool *fsal_stable,
                struct io_info *info);
fsal_status_t nullfs_seek2(struct fsal_obj_handle *obj_hdl,
               struct state_t *state,
               struct io_info *info);
fsal_status_t nullfs_io_advise2(struct fsal_obj_handle *obj_hdl,
                struct state_t *state,
                struct io_hints *hints);
fsal_status_t nullfs_commit2(struct fsal_obj_handle *obj_hdl, off_t offset,
                 size_t len);
fsal_status_t nullfs_lock_op2(struct fsal_obj_handle *obj_hdl,
                  struct state_t *state,
                  void *p_owner,
                  fsal_lock_op_t lock_op,
                  fsal_lock_param_t *req_lock,
                  fsal_lock_param_t *conflicting_lock);
fsal_status_t nullfs_close2(struct fsal_obj_handle *obj_hdl,
                struct state_t *state);

/* extended attributes management */
fsal_status_t nullfs_list_ext_attrs(struct fsal_obj_handle *obj_hdl,
                    unsigned int cookie,
                    fsal_xattrent_t *xattrs_tab,
                    unsigned int xattrs_tabsize,
                    unsigned int *p_nb_returned,
                    int *end_of_list);
fsal_status_t nullfs_getextattr_id_by_name(struct fsal_obj_handle *obj_hdl,
                       const char *xattr_name,
                       unsigned int *pxattr_id);
fsal_status_t nullfs_getextattr_value_by_name(struct fsal_obj_handle *obj_hdl,
                          const char *xattr_name,
                          caddr_t buffer_addr,
                          size_t buffer_size,
                          size_t *p_output_size);
fsal_status_t nullfs_getextattr_value_by_id(struct fsal_obj_handle *obj_hdl,
                        unsigned int xattr_id,
                        caddr_t buffer_addr,
                        size_t buffer_size,
                        size_t *p_output_size);
fsal_status_t nullfs_setextattr_value(struct fsal_obj_handle *obj_hdl,
                      const char *xattr_name,
                      caddr_t buffer_addr, size_t buffer_size,
                      int create);
fsal_status_t nullfs_setextattr_value_by_id(struct fsal_obj_handle *obj_hdl,
                                            unsigned int xattr_id,
                                            caddr_t buffer_addr,
                                            size_t buffer_size);
fsal_status_t nullfs_remove_extattr_by_id(struct fsal_obj_handle *obj_hdl,
                                          unsigned int xattr_id);
fsal_status_t nullfs_remove_extattr_by_name(struct fsal_obj_handle *obj_hdl,
                                            const char *xattr_name);
