#include "file_manager.h"

namespace dark {

namespace b_plus {

template <size_t __n>
struct string {
    char str[__n];
    
    string() noexcept { str[0] = '\0'; };

    string(const char *rhs) noexcept { strcpy(str,rhs); }

    string(const string &rhs) = default;

    string &operator =(const string &rhs) = default;

    const char *base() const noexcept { return str; }
};

template <size_t __n>
inline bool operator < (const string <__n> &lhs,const string <__n> &rhs) 
noexcept { return strcmp(lhs.base(),rhs.base()) < 0; }


using key_t = string <12>;
using   T   = int;
using key_comp = Compare <key_t>;
using val_comp = Compare   <T>;


constexpr int TABLE_SIZE = 1019;
constexpr int CACHE_SIZE = 300;
constexpr int BLOCK_SIZE = 3;
constexpr int AMORT_SIZE = BLOCK_SIZE * 2 / 3;
constexpr int SPLIT_SIZE = (BLOCK_SIZE + 1) / 2;
constexpr int  MAX_SIZE  = 300000;


class tree {
  private: /* Struct and using part. */

    /* Trivial key-value pair class. */
    struct value_t {
        key_t key; /*  Key.  */
        T     val; /* Value. */
        inline void copy(const key_t &__k,const T &__v) 
        { key = __k; val = __v; }
    };

    /* Tuple of value and index and count. */
    struct tuple_t {
        value_t v; /* Largest pair of target node. */
        header head;  /* A small header. */

        /* Copying header info and value. */
        inline void copy(header __h,const value_t &__v)
        { head = __h; v = __v;}

        /* Only copying key and value. */
        inline void copy(const key_t &key,const T &val)
        { v.copy(key,val); }
    };

    /* Index node trivial calss */
    struct node : header {
        header first; /* First header info. */
        tuple_t data[BLOCK_SIZE]; /* One more space for better performance. */
        /* Return head info of x-th node. x should be in [0,count] */
        inline header &head(int x) { return x ? data[x - 1].head : first; }
    };

    using node_file_t =
            file_manager <
                node,
                TABLE_SIZE,
                CACHE_SIZE
            >;
    using visitor = typename node_file_t::visitor;

   private: /* Data part. */

    [[no_unique_address]] key_comp k_comp; /*  Key  compare function. */
    [[no_unique_address]] val_comp v_comp; /* Value compare function. */

    std::pair <file_state,node> __root_pair; /* Do not use it directly. */

    file_state &root_state() { return __root_pair.first; }
    node &root() { return __root_pair.second; }

    node_file_t file;
    visitor cache_pointer;

   private:

    /**
     * @brief Search in [l,r) for ans location,
     * where A[ans - 1] < val < A[ans] .
     * 
     * @param data The array of the pair_t.
     * @param key  Key of the pair.
     * @param val  Value of the pair.
     * @return  Ans in [l,r] if found. || ~Ans if existing identical pair.
     */
    int binary_search(tuple_t *data,const key_t &key,const T &val,int l,int r) {
        while(l != r) { /* Find in [l,r) */
            int mid = (l + r) >> 1;

            int cmp = k_comp(key,data[mid].v.key);
            if(!cmp) cmp = v_comp(val,data[mid].v.val);

            if(cmp > 0) l = mid + 1;
            else if(cmp < 0) r = mid;
            else return ~mid;
        } return l;
    }

    /* Search the outer node. */
    int search(visitor pointer,const key_t &key,const T &val) 
    { return binary_search(pointer->data,key,val,0,pointer->count); }

    /* Get pointer for node at x position. */
    inline visitor get_pointer(int x)
    { return x ? file.get_object(x) : visitor{&__root_pair}; }

    /* Allocate one node. */
    inline visitor allocate() { return file.allocate(); }

    /* Insert into an empty tree. */
    void insert_root(const key_t &key,const T &val) {
        /* Allocate one node at outer file. */
        visitor pointer = allocate();

        /* Modify root information.  */
        root_state().modify();
        root().count = 1;
        root().head(0).set_index(pointer.index(),node_type::OUTER);
        root().head(0).count = 0;

        /* Modify new node information. */
        pointer.modify();
        pointer->set_index(MAX_SIZE,node_type::OUTER);
        pointer->count = 0;
        pointer->data[0].copy(key,val);
    }

    /* Split the root node */
    void split_root() {

    }

    /**
     * @brief Split at pointer's x-th son.
     * 
     * @param pointer Pointer of father node.
     * @param x       The subscript of the node to split.
     * @return 0 if split failed || 1 if split succeeded
     */
    bool split_node(visitor pointer,int x) {
        throw; // This won't happen
    }

   
    /* Amortize part of the pre node to the next node. */
    void amortize_prev(visitor prev,visitor next) {
        throw;
    }

    /* Amortize part of the next node to the prev node. */
    void amortize_next(visitor prev,visitor next) {
        throw;
    }

    /**
     * @brief Tries to amortize when inserting.
     * 
     * @param pointer Pointer of father node.
     * @param x       The subscript of the node to split.
     * @return 0 if amortization failed || 1 if amortization succeeded
     */
    bool insert_amortize(visitor pointer,int x) {
        if(x != pointer->count && pointer->head(x + 1).count < AMORT_SIZE) {
            amortize_prev(cache_pointer,
                          get_pointer(pointer->head(x + 1).real_index()));
        } else if(x != 0 && pointer->head(x - 1).count < AMORT_SIZE) {
            amortize_next(get_pointer(pointer->head(x - 1).real_index()),
                          cache_pointer);
        } else return false; /* Amortization failure. */
        return true;         /* Amortization success. */
    }


    /**
     * @brief Insert at an outer node.
     * 
     * @param head Head of the outer node.
     * @param key  Key to be inserted.
     * @param val  Value to be inserted.
     * @return 1 if successfully inserted || 0 if need adjustion.
     */
    bool insert_outer(header &head,const key_t &key,const T &val) {
        visitor pointer = get_pointer(head.real_index());
        int x = search(pointer,key,val);
        if(x < 0) return true; /* Find exactly the node. */

        /* Modify the data first. */
        pointer.modify();
        mmove(pointer->data + x + 1,pointer->data + x,pointer->count - x);
        pointer->data[x].copy(key,val);
        pointer->count = ++head.count;

        /* Return whether the node should be modified. */
        if(pointer->count == BLOCK_SIZE) {
            cache_pointer = pointer;
            return false;
        } else return true;
    }

    /**
     * @brief Insert at an inner node.
     * 
     * @param head Head of the outer node.
     * @param key  Key to be inserted.
     * @param val  Value to be inserted.
     * @return 0 if no need to adjust || 1 if need to adjust
     */
    bool insert(header &head,const key_t &key,const T &val) {
        if(!head.is_inner()) return insert_outer(head,key,val);
        visitor pointer = get_pointer(head.real_index());
        int x = search(pointer,key,val);
        if(x < 0) return true; /* Find exactly the node. */

        /* Need to change. */
        if(insert(pointer->head(x),key,val)) return true; /* No need to change. */

        /* Current node might require modification. */
        if(insert_amortize(pointer,x)) return true;

        /* Split the node now. */
        return split_node(pointer,x);
    }


  public: /* Public functions. */

    tree() = delete;

    /* Initialize the tree. */
    tree(std::string path1) :
        file(path1 + ".dat",path1 + ".bin") {
        if(file.empty()) {
            file.skip_block();
            root_state().modify();
            root().set_index(0,node_type::INNER);
            root().count = 0;
        } else {
            file.read_object(root(),0);
            root_state().state = false;
        }
    }

    /* Update root info if modified. */
    ~tree() { if(root_state().is_modified()) file.write_object(root(),0); }

    /* Return whether the tree is empty. */
    bool empty() const noexcept { return !__root_pair.second.count; }

    /* Insert a key-value pair into the node. */
    void insert(const key_t &key,const T &val) {
        if(empty()) return insert_root(key,val);
        if(!insert(root(),key,val)) split_root();
    }

    /* Erase a key-value pair from the node. */
    void erase(const key_t &__k,const T &__v) {
        if(empty()) return;
    }

};

}

}


signed main() {
    dark::b_plus::tree t("a");
    t.insert("5",3);
    return 0;
}
