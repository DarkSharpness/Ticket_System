#include "file_manager.h"
#include <algorithm>
#include <vector>
#include <set>

namespace dark {

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

namespace b_plus {

using key_t = string <68>;
using   T   = int;
using key_comp = Compare <key_t>;
using val_comp = Compare   <T>;

constexpr int TABLE_SIZE = 10000;
constexpr int CACHE_SIZE = 100000; // NO LESS THAN tree_height * 2 + 2
constexpr int BLOCK_SIZE = 101;
constexpr int AMORT_SIZE = BLOCK_SIZE * 2 / 3;
constexpr int MERGE_SIZE = BLOCK_SIZE / 3;
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
        value_t v; /* Samllest pair of target node. */
        header head;  /* A small header. */
        /* Copying header info and value. */
        inline void copy(const value_t &__v,header __h)
        { head = __h; v = __v;}

        /* Copying header info and value. */
        inline void copy(const key_t &key,const T &val,header __h)
        { head = __h; v.copy(key,val);}

        /* Only copying key and value. */
        inline void copy(const key_t &key,const T &val)
        { v.copy(key,val); }
    };

    /* Index node trivial class */
    struct node : header {
        tuple_t data[BLOCK_SIZE + 1]; /* One more space for better performance. */
        /* Return head info of x-th node. x should be in [0,count] */
        inline header &head(int x) { return data[x].head; }

        inline int next() const noexcept { return real_index(); }

        inline void set_next(int index,dark::node_type flag)
        { return set_index(index,flag); }

        inline void set_next(int index)
        { return set_index(index,node_type(is_inner())); }
    };

    struct node_reader {
        inline void operator ()(std::fstream &__f,node &obj) {
            __f.read((char *)(&obj),sizeof(header) + BLOCK_SIZE * sizeof(tuple_t));
        }
    };

    struct node_writer {
        inline void operator ()(std::fstream &__f,const node &obj) {
            __f.write((const char *)&obj,sizeof(header) + BLOCK_SIZE * sizeof(tuple_t));
        }
    };

    using node_file_t =
            file_manager <
                node,
                TABLE_SIZE,
                CACHE_SIZE,
                node_reader,
                node_writer
            >;

    using visitor = typename node_file_t::visitor;

   private: /* Data part. */

    [[no_unique_address]] key_comp k_comp; /*  Key  compare function. */
    [[no_unique_address]] val_comp v_comp; /* Value compare function. */

    std::pair <file_state,node> __root_pair; /* Do not use it directly. */

    node_file_t file;
    visitor cache_pointer;

   private:

    /**
     * @brief Search in [l,r) for ans location,
     * where data[ans - 1] < val < data[ans] .
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

    /**
     * @brief Search in [l,r) for ans location,
     * where key[ans - 1] < val <= key[ans] .
     * 
     * @param data The array of the pair_t.
     * @param key  Key of the pair.
     * @return  Ans in [l,r] if found. || ~Ans if existing identical pair.
     */
    int lower_bound(tuple_t *data,const key_t &key,int l,int r) {
        while(l != r) { /* Find in [l,r) */
            int mid = (l + r) >> 1;
            if(k_comp(key,data[mid].v.key) > 0) l = mid + 1;
            else r = mid;
        } return l;
    }

    /* Use memmove to move data fast. */
    static inline void mmove(tuple_t *dst,const tuple_t *src,int count) noexcept 
    { memmove(dst,src,count * sizeof(tuple_t)); }

    file_state &root_state() { return __root_pair.first; }
    node &root() { return __root_pair.second; }

    /* Get pointer for node at x position. */
    inline visitor get_pointer(header head) {
        int x = head.real_index();
        return x ? file.get_object(x) : visitor{&__root_pair};
    }

    inline void recycle(visitor x) { return file.recycle(x.index()); }

    /* Allocate one node. */
    inline visitor allocate() { return file.allocate(); }

    /* Insert into an empty tree. */
    void insert_root(const key_t &key,const T &val) {
        /* Allocate one node at outer file. */
        visitor pointer = allocate();

        /* Modify root information.  */
        root_state().modify();
        root().count = 1;
        root().data[0].copy(key,val,{~pointer.index(),1});

        /* Modify new node information. */
        pointer.modify();
        pointer->set_next(MAX_SIZE,node_type::OUTER);
        pointer->count = 1;
        pointer->data[0].copy(key,val);
    }

    /* Split the root node */
    void split_root() {
        visitor prev = allocate();
        visitor next = allocate();

        /* Update next() of prev and next.  */
        prev->state = next.index();
        next->state = MAX_SIZE;

        /* Update prev and next count and move data. */
        prev->count = root().count >> 1;
        next->count = (root().count + 1) >> 1;
        mmove(prev->data,        root().data      ,prev->count);
        mmove(next->data,root().data + prev->count,next->count);

        /* Modify root part. */
        root().count = 2;

        root().head(0) = {prev.index(),prev->count};
        root().head(1) = {next.index(),next->count};

        root().data[1].v = next->data[0].v;
    }


    /**
     * @brief Split at pointer's x-th son.
     * Note that pointer->count remains unchanged.
     * 
     * @param pointer Pointer of father node.
     * @param    x    The subscript of the node to split.
     */
    void split_node(visitor pointer,int x) {
        visitor prev = cache_pointer;
        visitor next = allocate();

        /* Update next() of prev and next.  */
        next->state  = prev->state;
        prev->set_next(next.index());

        /* Update prev and next count and move data. */
        prev->count -= (next->count = prev->count >> 1);
        pointer->head(x).count = prev->count;
        mmove(next->data,prev->data + prev->count,next->count);

        /* Insert a next at (x + 1)-th position of pointer. */
        if(++x < pointer->count)
            mmove(pointer->data + x + 1,pointer->data + x,pointer->count - x);
        pointer->data[x].v     = next->data[0].v;
        pointer->head(x).count = next->count;
        pointer->head(x).state = prev->state;
    }


    /**
     * @brief Merge 2 nodes into previous one node and recycle the second one.
     * 
     * @param prev Previous node.
     * @param next Next     node.
     */
    void merge_node(visitor prev,visitor next) {
        /* Update pointer first. */
        prev.modify();

        /* Relink , update count and move data. */
        prev->state  = next->state;
        mmove(prev->data + prev->count,next->data,next->count);
        prev->count += next->count;

        /* This should never happen!!! */
        // if(prev->count >= BLOCK_SIZE) throw error("Wrongly merged!");
        /* Recyle nodes. */
        recycle(next);
    }


    /**
     * @brief Try merge at pointer's x-th son.
     * Note that pointer->count remains unchanged.
     * Fail only when pointer points to root node 
     * with less than 2 sons.
     * 
     * @param pointer Pointer of father node.
     * @param    x    The subscript of the node to split.
     */
    void erase_merge(visitor pointer,int x) {
        /* Of course , pointer must points to root now. */
        if(pointer->count == 1) {
            if(pointer.index() != 0) throw error("Erase merge!");
            else if(cache_pointer->count != 0) ++pointer->count;
            return;
        }
        /* Merge with smaller brother. */
        bool flag = x != pointer->count - 1;
        if(flag && x != 0) 
            flag = pointer->head(x - 1).count > pointer->head(x + 1).count;

        if(flag) { /* Merge with next node. */
            visitor prev = cache_pointer;
            visitor next = get_pointer(pointer->head(x + 1));
            merge_node(prev,next);
            mmove(pointer->data + x + 1,pointer->data + x + 2,pointer->count - x - 2);
            pointer->head(x).count = prev->count;
        } else {   /* Merge with prev node. */
            visitor prev = get_pointer(pointer->head(x - 1));
            visitor next = cache_pointer;
            merge_node(prev,next);
            mmove(pointer->data + x,pointer->data + x + 1,pointer->count - x - 1);
            pointer->head(x - 1).count = prev->count;
        }
    }


    /* Amortize part of the prev node to the next node. */
    inline void amortize_prev(visitor prev,visitor next) {
        prev.modify();
        next.modify();

        /* Move some of the data. */
        int delta = (prev->count - next->count) >> 1;
        mmove(next->data + delta,next->data,next->count);
        prev->count -= delta;
        next->count += delta;
        mmove(next->data,prev->data + prev->count,delta);
    }


    /* Amortize part of the next node to the prev node. */
    inline void amortize_next(visitor prev,visitor next) {
        prev.modify();
        next.modify();

        /* Move some of the data. */
        int delta = (next->count - prev->count) >> 1;
        mmove(prev->data + prev->count,next->data,delta);
        prev->count += delta;
        next->count -= delta;
        mmove(next->data,next->data + delta,next->count);
    }


    /**
     * @brief Tries to amortize when inserting.
     * 
     * @param pointer Pointer of father node.
     * @param x       The subscript of the node to split.
     * @return 0 if amortization failed || 1 if amortization succeeded
     */
    bool insert_amortize(visitor pointer,int x) {
        bool flag[2] =  {       
            x != pointer->count - 1 && pointer->head(x + 1).count < AMORT_SIZE,
                   x != 0           && pointer->head(x - 1).count < AMORT_SIZE
        };

        if(flag[0] && flag[1]) /* Amortize with smaller brother. */
            flag[pointer->head(x - 1).count > pointer->head(x + 1).count] = false;

        if(flag[1]) {
            visitor prev = get_pointer(pointer->head(x - 1));
            visitor next = cache_pointer;
            amortize_next(prev,next);
            pointer->head(x - 1).count = prev->count;
            pointer->head(x).count     = next->count;
            pointer->data[x].v         = next->data[0].v;
        } else if(flag[0]) {
            visitor prev = cache_pointer;
            visitor next = get_pointer(pointer->head(x + 1));
            amortize_prev(prev,next);
            pointer->head(x).count     = prev->count;
            pointer->head(x + 1).count = next->count;
            pointer->data[x + 1].v     = next->data[0].v;
        } else return false;
        return true;
    }


    /**
     * @brief Tries to amortize when inserting.
     * 
     * @param pointer Pointer of father node.
     * @param x       The subscript of the node to split.
     * @return 0 if amortization failed || 1 if amortization succeeded
     */
    bool erase_amortize(visitor pointer,int x) {
        bool flag[2] =  {
                   x != 0           && pointer->head(x - 1).count >= AMORT_SIZE,
            x != pointer->count - 1 && pointer->head(x + 1).count >= AMORT_SIZE
        };

        if(flag[0] && flag[1]) /* Amortize with larger brother. */
            flag[pointer->head(x - 1).count > pointer->head(x + 1).count] = false;

        if(flag[0]) {
            visitor prev = get_pointer(pointer->head(x - 1));
            visitor next = cache_pointer;
            amortize_prev(prev,next);
            pointer->head(x - 1).count = prev->count;
            pointer->head(x).count     = next->count;
            pointer->data[x].v         = next->data[0].v;
        } else if(flag[1]) {
            visitor prev = cache_pointer;
            visitor next = get_pointer(pointer->head(x + 1));
            amortize_next(prev,next);
            pointer->head(x).count     = prev->count;
            pointer->head(x + 1).count = next->count;
            pointer->data[x + 1].v     = next->data[0].v;
        } else return false;
        return true;
    }


    /**
     * @brief Insert at an outer node.
     * 
     * @param head Head of the outer node.
     * @param key  Key to be inserted.
     * @param val  Value to be inserted.
     * @return 1 if successfully inserted , which means father node is modified 
     *      || 0 if nothing is changed
     */
    bool insert_outer(header &head,const key_t &key,const T &val) {
        /* Binary searching. */
        visitor pointer = get_pointer(head);
        // if(head.count != pointer->count) throw error("outer insert");
        int x = binary_search(pointer->data,key,val,0,head.count);
        if(x < 0) return false; /* Find exactly the node. */

        /* Data will be modified. */
        pointer.modify();

        /* Insert the key-value pair into the node. */
        mmove(pointer->data + x + 1,pointer->data + x,head.count - x);
        pointer->data[x].copy(key,val);
        head.count = ++pointer->count;

        /* Move the pointer to cache. */
        cache_pointer = pointer;
        return true;
    }


    /**
     * @brief Insert at an inner node.
     * 
     * @param head Head of the outer node.
     * @param key  Key to be inserted.
     * @param val  Value to be inserted.
     * @return 1 if successfully inserted , which means father node is modified
     *      || 0 if nothing is changed
     */
    bool insert(header &head,const key_t &key,const T &val) {
        /* If outer file , start insertion. */
        if(!head.is_inner()) return insert_outer(head,key,val);

        /* Binary searching. */
        visitor pointer = get_pointer(head);
        // if(head.count != pointer->count) throw error("inner insert");
        int x = binary_search(pointer->data,key,val,0,head.count);
        if(x < 0) return false; /* Find exactly the node. */
        else if(x > 0) --x;
        else { /* The smallest element in the block. */
            pointer.modify();
            pointer->data[0].copy(key,val);
        }

        /* Insert into node now. */
        if(!insert(pointer->head(x),key,val)) return false;

        /* Need to adjust the parent now. */
        pointer.modify();

        /* Son is not full , so nothing is done to this node.*/
        if(cache_pointer->count <= BLOCK_SIZE) return false;

        /* Current node might require modification. */
        if(insert_amortize(pointer,x)) return false;

        /* Split the node now. */
        split_node(pointer,x);
        head.count = ++pointer->count;

        /* Move the pointer to cache. */
        cache_pointer = pointer;
        return true;
    }


    /**
     * @brief Insert at an outer node.
     * 
     * @param head Head of the outer node.
     * @param key  Key to be inserted.
     * @param val  Value to be inserted.
     * @return 1 if successfully inserted , which means father node is modified 
     *      || 0 if nothing is changed
     */
    bool erase_outer(header &head,const key_t &key,const T &val) {
        /* Binary searching. */
        visitor pointer = get_pointer(head);
        // if(head.count != pointer->count) throw error("outer erase");
        int x = ~binary_search(pointer->data,key,val,0,head.count);
        if(x < 0) return false; /* Don't find exactly the node. */

        /* Data will be modified. */
        pointer.modify();

        /* Insert the key-value pair into the node. */
        mmove(pointer->data + x,pointer->data + x + 1,head.count - x - 1);
        head.count = --pointer->count;

        /* Move the pointer to cache. */
        cache_pointer = pointer;
        return true;
    }


    /**
     * @brief Insert at an inner node.
     * 
     * @param head Head of the outer node.
     * @param key  Key to be inserted.
     * @param val  Value to be inserted.
     * @return 1 if successfully inserted , which means father node is modified
     *      || 0 if nothing is changed
     */
    bool erase(header &head,const key_t &key,const T &val) {
        /* If outer file , start insertion. */
        if(!head.is_inner()) return erase_outer(head,key,val);

        /* Binary searching. */
        visitor pointer = get_pointer(head);
        // if(head.count != pointer->count) throw error("inner erase");
    
        int x = binary_search(pointer->data,key,val,0,head.count);

        bool flag = false;       /* Whether to update the smallest. */
        if(x == 0) return false; /* Smaller than the smallest node. */
        else if(x > 0) --x;      /* Move the position */
        else x = ~x,flag = true; /* Find exactly the smallest in the node. */

        /* Erase from the node now. */
        if(!erase(pointer->head(x),key,val)) return false;

        /* Need to adjust the parent now. */
        pointer.modify();

        /* If exactly smallest , related data will be updated. */
        if(flag) pointer->data[x].v = cache_pointer->data[0].v;

        /* Son is not that empty , only when data[0] is updated. */
        if(cache_pointer->count > MERGE_SIZE) return flag && !x;

        /* Current node might require modification. */
        if(erase_amortize(pointer,x)) return flag && !x;

        /* Merge the node's son now. */
        erase_merge(pointer,x);
        head.count    = --pointer->count;
        cache_pointer = pointer;
        return true;
    }


    /* DEBUG USE ONLY! */
    const value_t &print_outer(header head) {
        visitor pointer = get_pointer(head);
        // if(head.count != pointer->count) throw error("Outer Mis-match");

        std::cout << "Outer block "  << head.real_index() << " :\n";
        for(int i = 0 ; i != head.count ; ++i)
            std::cout << "Leaf " << i << ": || key: "
                      << pointer->data[i].v.key.str
                      << " || value: "
                      << pointer->data[i].v.val
                      << " ||\n";
        std::cout << "Next index x: " << pointer->next();
        std::cout << "\n--------------------------------\n";

        return pointer->data[0].v;
    }

    const value_t &check_outer(header head) {
        visitor pointer = get_pointer(head);
        if(head.count != pointer->count) throw error("Outer Mis-match");
        return pointer->data[0].v;
    }

    const value_t &check(header head) {
        if(!head.is_inner()) return check_outer(head);
        visitor pointer = get_pointer(head);
        if(head.count != pointer->count)
            throw error("Inner Mis-Match!!!");
        for(int i = 0 ; i != head.count ; ++i) {
            auto &&temp = check(pointer->head(i));
            if(k_comp(temp.key,pointer->data[i].v.key) ||
               v_comp(temp.val,pointer->data[i].v.val)) {
                throw error("Pair dismatch");
            }
        }
        return pointer->data[0].v;
    }

    const value_t &get_array(header head,std::vector <int> &t,std::set <int> &s) {
        if(head.real_index() && s.count(head.real_index())) throw error("fucked");
        s.insert(head.real_index());
        if(!head.is_inner()) return check_outer(head);
        visitor pointer = get_pointer(head);
        if(head.count != pointer->count)
            throw error("Inner Mis-Match!!!");
        for(int i = 0 ; i != head.count ; ++i) {
            auto &&temp = get_array(pointer->head(i),t,s);
            t.push_back(pointer->head(i).real_index());
            if(k_comp(temp.key,pointer->data[i].v.key) ||
               v_comp(temp.val,pointer->data[i].v.val)) {
                throw error("Pair dismatch");
            }
        }
        return pointer->data[0].v;
    }

    /* DEBUG USE ONLY! */
    const value_t &print(header head) {
        if(!head.is_inner()) return print_outer(head);
        visitor pointer = get_pointer(head);
        if(head.count != pointer->count)
            throw error("Inner Mis-Match!!!");

        std::cout << "Inner block "  << head.real_index() << " :\n";
        for(int i = 0 ; i != head.count ; ++i)
            std::cout << "Son " << i << ": || index: "
                      << pointer->head(i).real_index()
                      << " || key: "
                      << pointer->data[i].v.key.str
                      << " || value: "
                      << pointer->data[i].v.val
                      << " ||\n";
        std::cout << "Next index x: " << pointer->next();
        std::cout << "\n--------------------------------\n";

        for(int i = 0 ; i != head.count ; ++i) {
            auto &&temp = print(pointer->head(i));
            if(k_comp(temp.key,pointer->data[i].v.key) ||
               v_comp(temp.val,pointer->data[i].v.val)) {
                throw error("Pair dismatch");
            }
        } return pointer->data[0].v;
    }

  public: /* Public functions. */


    using return_list = dark::trivial_array <T>;


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
        /* Empty Tree special case. */
        if(empty()) return insert_root(key,val);
        /* Nothing should be done to root node case. */
        if(!insert(root(),key,val)) return;
        /* When the node under root is too full. */
        if(root().count > BLOCK_SIZE) split_root();
    }


    /* Erase a key-value pair from the node. */
    void erase(const key_t &key,const T &val) {
        /* Empty Tree special case. */
        if(empty()) return;
        /* Nothing should be done to root node case. */
        else erase(root(),key,val);
    }


    /* Find all value-type binded to key. */
    void find(const key_t &key,return_list &v) {
        if(empty()) return;
        header head = root();
        /* Find the real inner node. */
        while(head.is_inner()) {
            visitor pointer = get_pointer(head);
            int x = lower_bound(pointer->data + 1,key,0,head.count - 1);
            head = pointer->head(x);
        }
        /* The real outer node. */
        visitor pointer = get_pointer(head);
        int x = lower_bound(pointer->data,key,0,head.count);
        /* Find in the first block. */
        while(x != head.count) {
            if(k_comp(key,pointer->data[x].v.key)) return;
            v.push_back(pointer->data[x++].v.val);
        }
        /* Find in the second block. */
        while(pointer->next() != MAX_SIZE) {
            pointer = get_pointer(*pointer); x = 0;
            while(x != pointer->count){
                if(k_comp(key,pointer->data[x].v.key)) return;
                v.push_back(pointer->data[x++].v.val);
            }
        }
    }

    void strong_check() {
        static std::vector <int> t;
        t.clear();
        t.reserve(file.bin.total);
        t.push_back(0);
        for(auto iter : file.bin.bin_array)
            t.push_back(iter);
        std::sort(t.begin(),t.end());
        std::set <int> s;
        for(auto iter : t) s.emplace_hint(s.end(),iter);
        get_array(root(),t,s);
        std::sort(t.begin(),t.end());
        for(int i = 0 ; i != (int)t.size() ; ++i)
            if(t[i] != i) throw error("BLOCK LEAK");
        if(t.size() != file.bin.total) throw error("Size dismatch");
    }

    /* DEBUG USE ONLY! */
    void check_function() { if(!empty()) return (void)check(root()); }

    void print_function() { if(!empty()) return (void)print(root()); }
};


}

}


