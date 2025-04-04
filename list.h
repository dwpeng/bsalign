#ifndef __LIST_RJ_H
#define __LIST_RJ_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "mem_share.h"

/**
 * Heap macros
 */

//ary, size and cap must be explict variable, not expression
#define array_heap_push(ary, size, cap, e_type, id, cmp_expr)\
do {	\
	e_type *_ary_, _pp_, _p_;	\
	_ary_ = (e_type*)(ary);	\
	_p_ = (e_type)(id);	\
	size_t i, j;	\
	i = (size);	\
	e_type a, b;	\
	if((size_t)((size) + 1) > (size_t)(cap)){	\
		if((size_t)((size) + 1) < 0xFFFFFFFFU){	\
			(cap) = roundup_power2((size) + 1);	\
		} else {	\
			(cap) = (((size) + 1 + 0xFFFFFFFLLU - 1LLU) / 0xFFFFFFFLLU) * 0xFFFFFFFLLU;	\
		}	\
		_ary_ = realloc(_ary_, sizeof(e_type) * (cap));	\
		if(_ary_ == NULL){	\
			fprintf(stderr, " -- Out of memory, try to allocate %llu bytes in %s, -- %s:%d --\n", (unsigned long long)(sizeof(e_type) * (cap)), __FUNCTION__, __FILE__, __LINE__);	\
			print_backtrace(stderr, 20);	\
			exit(1);	\
		}	\
		ary = (typeof(ary))_ary_;	\
	}	\
	_ary_[(size)++] = _p_;	\
	while(i){	\
		j = (i - 1) >> 1;	\
		a = _ary_[i]; b = _ary_[j];	\
		if((cmp_expr) >= 0) break;	\
		_pp_ = _ary_[i]; _ary_[i] = _ary_[j]; _ary_[j] = _pp_;	\
		i = j;	\
	}	\
} while(0)

// please don't call it on list->aligned > SYS_ALIGNED_BASE(8) or list->n_head > 0
#define list_heap_push(list, id, cmp_expr) array_heap_push((list)->buffer, (list)->size, (list)->cap, typeof(*(list)->buffer), id, cmp_expr)

#define array_heap_remove(ary, len, cap, e_type, _idx, cmp_expr)\
({	\
	e_type *_ary_, _pp_, _ret_;	\
	_ary_ = (e_type*)(ary);	\
	size_t swap, idx;	\
	idx = (size_t)(_idx);	\
	_ret_ = _ary_[idx];	\
	_ary_[idx] = _ary_[--(len)];	\
	e_type a, b;	\
	while((size_t)((idx << 1) + 1) < (size_t)(len)){	\
		swap = idx;	\
		a = _ary_[swap]; b = _ary_[(idx << 1) + 1];	\
		if((cmp_expr) > 0) swap = (idx << 1) + 1;	\
		if(((idx << 1) + 2) < (size_t)(len)){	\
			a = _ary_[swap]; b = _ary_[(idx << 1) + 2];	\
			if((cmp_expr) > 0) swap = (idx << 1) + 2;	\
		}	\
		if(swap == idx) break;	\
		_pp_ = _ary_[idx]; _ary_[idx] = _ary_[swap]; _ary_[swap] = _pp_;	\
		idx = swap;	\
	}	\
	_ret_;	\
})

#define list_heap_remove(list, _idx, cmp_expr) array_heap_remove((list)->buffer, (list)->size, (list)->cap, typeof(*(list)->buffer), _idx, cmp_expr)

#define array_heap_replace(ary, len, cap, e_type, _idx, _val, cmp_expr)\
do {	\
	e_type *_ary_, _pp_;	\
	size_t swap, idx;	\
	idx = (size_t)(_idx);	\
	_ary_[idx] = _val;	\
	e_type a, b;	\
	while((size_t)((idx << 1) + 1) < (size_t)(len)){	\
		swap = idx;	\
		a = _ary_[swap]; b = _ary_[(idx << 1) + 1];	\
		if((cmp_expr) > 0) swap = (idx << 1) + 1;	\
		if(((idx << 1) + 2) < (size_t)(len)){	\
			a = _ary_[swap]; b = _ary_[(idx << 1) + 2];	\
			if((cmp_expr) > 0) swap = (idx << 1) + 2;	\
		}	\
		if(swap == idx) break;	\
		_pp_ = _ary_[idx]; _ary_[idx] = _ary_[swap]; _ary_[swap] = _pp_;	\
		idx = swap;	\
	}	\
} while(0)

#define list_heap_replace(list, _idx, _val, cmp_expr) array_heap_replace((list)->buffer, (list)->size, (list)->cap, typeof(*(list)->buffer), _idx, _val, cmp_expr)

#define array_heap_pop(ary, len, cap, e_type, cmp_expr)	\
({	\
	e_type _ret_;	\
	if(len){ _ret_ = ary[0]; array_heap_remove(ary, len, cap, e_type, 0, cmp_expr); }	\
	else memset(&_ret_, 0xFFU, sizeof(e_type));	\
	_ret_;	\
})

// make sure you have included "sort.h"
#define sort_list(list, cmpgt_expr) sort_array((list)->buffer, (list)->size, typeof((list)->buffer[0]), cmpgt_expr)

/**
 * List
 */

#define LIST_MAX_N_HEAD	15

#define define_list_core(list_type, e_type, size_type, inc_size)	\
	\
typedef struct {	\
	e_type* buffer;	\
	size_type size;	\
	size_type cap;	\
	size_type mem_zero:1, n_head:6, aligned:6, off:sizeof(size_type)*8-13;	\
} list_type;	\
	\
static inline size_t list_type##_obj_desc_cnt(void *list, int idx){	\
	if(idx == 0) return ((list_type*)list)->size * sizeof(e_type);	\
	else return 0;	\
}	\
	\
static inline void list_type##_obj_desc_post_load(void *obj, size_t aux_data){	\
	list_type *list;	\
	UNUSED(aux_data);	\
	list = (list_type*)obj;	\
	list->cap = list->size;	\
}	\
	\
static const obj_desc_t list_type##_obj_desc = {.tag = TOSTR(list_type), .size = sizeof(list_type), .n_child = 1, .mem_type = {1}, .addr = {offsetof(list_type, buffer)}, .desc = {(struct obj_desc_t*)&OBJ_DESC_DATA}, .cnt = list_type##_obj_desc_cnt, .post = list_type##_obj_desc_post_load};	\
	\
static inline void adv_##list_type##_init(list_type *list, size_type init_size, int mem_zero, int aligned_base, u1i n_head){	\
	if(init_size == 0) init_size = 2;	\
	list->size = 0;	\
	list->off  = 0;	\
	list->cap  = init_size;	\
	list->mem_zero = mem_zero;	\
	list->n_head = n_head;	\
	list->aligned = aligned_base;	\
	if(list->mem_zero){	\
		list->buffer = (e_type*)aligned_calloc((list->cap + list->n_head) * sizeof(e_type), aligned_base) + list->n_head;	\
	} else {	\
		list->buffer = (e_type*)aligned_malloc((list->cap + list->n_head) * sizeof(e_type), aligned_base) + list->n_head;	\
	}	\
}	\
	\
static inline list_type* adv_init_##list_type(size_type init_size, int mem_zero, int aligned_base, u4i n_head){	\
	list_type *list = (list_type*)malloc(sizeof(list_type));	\
	adv_##list_type##_init(list, init_size, mem_zero, aligned_base, n_head);	\
	return list;	\
}	\
	\
static inline list_type* init_##list_type(size_type init_size){	\
	return adv_init_##list_type(init_size, 0, SYS_ALIGNED_BASE, 0);	\
}	\
	\
static inline void list_type##_init(list_type *list, size_type init_size){	\
	adv_##list_type##_init(list, init_size, 0, SYS_ALIGNED_BASE, 0);	\
}	\
	\
static inline int head_sl_##list_type(list_type *list, size_type len){	\
	if(list->n_head < len) return 0;	\
	list->buffer -= len;	\
	list->n_head -= len;	\
	list->cap    += len;	\
	if(list->size) list->size += len;	\
	return 1;	\
}	\
	\
static inline int head_sr_##list_type(list_type *list, size_type len){	\
	if(list->cap < len) return 0;	\
	list->buffer += len;	\
	list->n_head += len;	\
	list->cap    -= len;	\
	if(list->size > len) list->size -= len;	\
	else list->size = 0;	\
	return 1;	\
}	\
	\
static inline void renew_##list_type(list_type *list, size_type init_size){	\
	if(list->buffer) aligned_free(list->buffer - list->n_head, list->aligned);	\
	adv_##list_type##_init(list, init_size, list->mem_zero, list->aligned, list->n_head);	\
}	\
	\
static inline size_type count_##list_type(list_type *list){ return list->size; }	\
	\
static inline void clear_##list_type(list_type *list){ list->size = 0; list->off = 0; }	\
	\
static inline void zeros_##list_type(list_type *list){ memset(list->buffer - list->n_head, 0, (list->size + list->n_head) * sizeof(e_type)); }	\
	\
static inline void all_zeros_##list_type(list_type *list){ memset(list->buffer - list->n_head, 0, (list->cap + list->n_head) * sizeof(e_type)); }	\
	\
static inline void encap_##list_type(list_type *list, size_type n){	\
	list->cap = encap_list((void**)&(list->buffer), sizeof(e_type), list->size, list->cap, n, list->mem_zero, list->n_head, list->aligned);	\
}	\
	\
static inline void recap_##list_type(list_type *list, size_type n){	\
	if((size_t)n == (size_t)list->cap) return;	\
	list->cap = n;	\
	if(list->size > n) list->size = n;	\
	list->buffer = aligned_realloc(list->buffer - list->n_head, (list->size + list->n_head) * sizeof(e_type), (list->cap + list->n_head) * sizeof(e_type), list->aligned) + list->n_head;	\
}	\
	\
static inline void pack_##list_type(list_type *list){	\
	return recap_##list_type(list, list->size);	\
}	\
	\
static inline void encap_and_zeros_##list_type(list_type *list, size_type n){	\
	if(((size_t)list->size) + ((size_t)n) <= (size_t)list->cap){	\
	} else {	\
		if((size_t)(list->size + n) != ((size_t)list->size) + ((size_t)n)){	\
			fprintf(stderr, " -- elements size exceed %s's data type %s in %s -- %s:%d --\n", #list_type, #size_type, __FUNCTION__, __FILE__, __LINE__);	\
			print_backtrace(stderr, 20);	\
			fflush(stderr);	\
			exit(1);	\
		}	\
		while(list->size + n > list->cap){	\
			if(list->cap < inc_size){	\
				list->cap <<= 1;	\
			} else {	\
				list->cap += inc_size;	\
			}	\
		}	\
		list->buffer = aligned_realloc(list->buffer - list->n_head, (list->size + list->n_head) * sizeof(e_type), (list->cap + list->n_head) * sizeof(e_type), list->aligned) + list->n_head;	\
	}	\
	memset(list->buffer + list->size, 0, n * sizeof(e_type));	\
}	\
	\
static inline void clear_and_encap_##list_type(list_type *list, size_type n){	\
	list->size = 0;	\
	list->off  = 0;	\
	encap_##list_type(list, n);	\
}	\
	\
static inline void clear_and_inc_##list_type(list_type *list, size_type n){	\
	list->size = 0;	\
	list->off  = 0;	\
	encap_##list_type(list, n);	\
	list->size = n;	\
}	\
	\
static inline void resize_##list_type(list_type *list, size_type size){	\
	clear_and_inc_##list_type(list, size);	\
}	\
	\
static inline void dec_##list_type(list_type *list, size_type size){	\
	if(size > count_##list_type(list)) size = count_##list_type(list);	\
	list->size -= size;	\
}	\
	\
static inline void set_##list_type##_size(list_type *list, size_type size){ list->size = size; }	\
	\
static inline void inc_##list_type(list_type *list, size_type size){	\
	encap_##list_type(list, size);	\
	list->size += size;	\
}	\
	\
static inline void lazy_push_##list_type(list_type *list, e_type e){ list->buffer[list->size ++] = e; }	\
	\
static inline void push_##list_type(list_type *list, e_type e){	\
	encap_##list_type(list, 1);	\
	list->buffer[list->size++] = e;	\
}	\
	\
static inline e_type ring_get_##list_type(list_type *list, size_type idx){	\
	idx = (list->off + idx) % list->cap;	\
	return list->buffer[idx];	\
}	\
	\
static inline e_type* ring_ref_##list_type(list_type *list, size_type idx){	\
	idx = (list->off + idx) % list->cap;	\
	return list->buffer + idx;	\
}	\
	\
static inline void ring_push_##list_type(list_type *list, e_type e){	\
	size_type idx;	\
	idx = (list->off + list->size) % list->cap;	\
	list->buffer[idx] = e;	\
	if(list->size < list->cap){	\
		list->size ++;	\
	}	\
}	\
	\
static inline e_type* ring_peer_##list_type(list_type *list){	\
	if(list->size){	\
		return list->buffer + list->size - 1;	\
	} else return NULL;	\
}	\
	\
static inline e_type* ring_pop_##list_type(list_type *list){	\
	size_type idx;	\
	if(list->size){	\
		list->size --;	\
		idx = (list->off + list->size) % list->cap;	\
		return list->buffer + idx;	\
	} else return NULL;	\
}	\
	\
static inline void ring_unshift_##list_type(list_type *list, e_type e){	\
	list->off = (list->off + list->cap - 1) % list->cap;	\
	list->buffer[list->off] = e;	\
	if(list->size < list->cap){	\
		list->size ++;	\
	}	\
}	\
	\
static inline e_type* ring_shift_##list_type(list_type *list){	\
	size_type idx;	\
	if(list->size){	\
		list->size --;	\
		idx = list->off;	\
		list->off = (list->off + 1) % list->cap;	\
		return list->buffer + idx;	\
	} else return NULL;	\
}	\
	\
static inline int fpush_##list_type(list_type *list, FILE *inp){	\
	int ret;	\
	encap_##list_type(list, 1);	\
	ret = fread(list->buffer + list->size, sizeof(e_type), 1, inp);	\
	if(ret == 1) list->size ++;	\
	return ret;	\
}	\
	\
static inline e_type* peer_##list_type(list_type *list){	\
	if(count_##list_type(list)){	\
		return list->buffer + list->size - 1;	\
	} else return NULL;	\
}	\
	\
static inline e_type* head_##list_type(list_type *list){	\
	if(list->size) return list->buffer;	\
	else return NULL;	\
}	\
	\
static inline e_type* tail_##list_type(list_type *list){	\
	if(list->size) return list->buffer + list->size - 1;	\
	else return NULL;	\
}	\
static inline int pop_##list_type(list_type *list, e_type*e){	\
	if(count_##list_type(list)){	\
		list->size --;	\
		*e = list->buffer[list->size];	\
		return 1;	\
	} else return 0;	\
}	\
	\
static inline int fpop_##list_type(list_type *list, FILE *oup){	\
	if(list->size){	\
		fwrite(list->buffer + list->size - 1, sizeof(e_type), 1, oup);	\
		list->size --;	\
		return 1;	\
	} else {	\
		return 0;	\
	}	\
}	\
	\
static inline void insert_##list_type(list_type *list, size_type idx, e_type e){	\
	if(idx > list->size) return;	\
	encap_##list_type(list, 1);	\
	if(idx == list->size){	\
		list->buffer[list->size] = e;	\
	} else {	\
		memmove(list->buffer + idx + 1, list->buffer + idx, (list->size - idx) * sizeof(e_type));	\
		list->buffer[idx] = e;	\
	}	\
	list->size ++;	\
}	\
	\
static inline void insert_array_##list_type(list_type *list, size_type idx, e_type *es, size_type size){	\
	if(idx > list->size) return;	\
	encap_##list_type(list, size);	\
	if(idx == list->size){	\
	} else {	\
		memmove(list->buffer + idx + size, list->buffer + idx, (list->size - idx) * sizeof(e_type));	\
	}	\
	memcpy(list->buffer + idx, es, size * sizeof(e_type));	\
	list->size += size;	\
}	\
	\
static inline void remove_##list_type(list_type *list, size_type idx){	\
	if(idx >= list->size) return;	\
	if(idx + 1 < list->size){	\
		memmove(list->buffer + idx, list->buffer + idx + 1, (list->size - idx - 1) * sizeof(e_type));	\
	}	\
	list->size --;	\
}	\
	\
static inline void remove_array_##list_type(list_type *list, size_type off, size_type len){	\
	if(off >= list->size) return;	\
	if(off + len < list->size){	\
		memmove(list->buffer + off, list->buffer + off + len, (list->size - off - len) * sizeof(e_type));	\
		list->size -= len;	\
	} else { \
		list->size = off;	\
	}	\
}	\
	\
static inline void set_##list_type(list_type *list, size_type idx, e_type e){ list->buffer[idx] = e; }	\
	\
static inline e_type get_##list_type(list_type *list, size_type idx){ return list->buffer[idx]; }	\
	\
static inline e_type* ref_##list_type(list_type *list, size_type idx){ return list->buffer + idx; }	\
	\
static inline size_type offset_##list_type(list_type *list, e_type *e){ return e - list->buffer; }	\
	\
static inline e_type* next_ref_##list_type(list_type *list){ encap_##list_type(list, 1); list->size ++; return list->buffer + list->size - 1; }	\
	\
static inline e_type* ref_next_##list_type(list_type *list){ list->size ++; return list->buffer + list->size - 1; }	\
	\
static inline e_type* as_array_##list_type(list_type *list){ return list->buffer; }	\
	\
static inline void reverse_##list_type(list_type *list){	\
	size_type i, j;	\
	e_type t;	\
	if(count_##list_type(list) == 0) return;	\
	i = 0;	\
	j = count_##list_type(list) - 1;	\
	while(i < j){	\
		t = get_##list_type(list, i);	\
		set_##list_type(list, i, get_##list_type(list, j));	\
		set_##list_type(list, j, t);	\
		i ++;	\
		j --;	\
	}	\
}	\
	\
static inline void sub_reverse_##list_type(list_type *list, size_type beg, size_type end){	\
	size_type i, j;	\
	e_type t;	\
	if(end <= beg) return;	\
	i = beg;	\
	j = end - 1;	\
	while(i < j){	\
		t = get_##list_type(list, i);	\
		set_##list_type(list, i, get_##list_type(list, j));	\
		set_##list_type(list, j, t);	\
		i ++;	\
		j --;	\
	}	\
}	\
	\
static inline void append_##list_type(list_type *list1, list_type *list2){	\
	encap_##list_type(list1, count_##list_type(list2));	\
	memcpy(list1->buffer + list1->size, list2->buffer, sizeof(e_type) * list2->size);	\
	list1->size += list2->size;	\
}	\
	\
static inline void append_array_##list_type(list_type *list1, e_type *ary, size_type size){	\
	if(size == 0) return;	\
	encap_##list_type(list1, size);	\
	memcpy(list1->buffer + list1->size, ary, sizeof(e_type) * size);	\
	list1->size += size;	\
}	\
	\
static inline void free_##list_type(list_type *list){ if(list){ aligned_free(list->buffer - list->n_head, list->aligned); free(list); } }	\
	\
static inline void list_type##_free(list_type *list){ aligned_free(list->buffer - list->n_head, list->aligned); list->buffer = NULL; }	\

#define define_list_ext(list_type, e_type, size_type, cmp_func)	\
static inline size_type delete_##list_type(list_type *list, e_type e){	\
	size_type i, ret;	\
	ret = 0;	\
	for(i=list->size;i>0;i--){	\
		if(cmp_func(list->buffer[i-1], e, NULL) == 0){	\
			if(i < list->size){	\
				memmove(list->buffer + i - 1, list->buffer + i, (list->size - i) * sizeof(e_type));	\
			}	\
			list->size --;	\
			ret ++;	\
		}	\
	}	\
	return ret;	\
}	\
	\
static inline size_type occ_##list_type(list_type *list, e_type e){	\
	size_type i, n;	\
	for(i=0,n=0;i<list->size;i++){	\
		if(cmp_func(list->buffer[i], e, NULL) == 0) n++;	\
	}	\
	return n;	\
}	\
	\
static inline size_type replace_##list_type(list_type *list, e_type from, e_type to){	\
	size_type i, ret;	\
	ret = 0;	\
	for(i=0;i<list->size;i++){	\
		if(cmp_func(list->buffer[i], from, NULL) == 0){	\
			list->buffer[i] = to;	\
			ret ++;	\
		}	\
	}	\
	return ret;	\
}	\
	\
static inline size_type locate_##list_type(list_type *list, e_type e, size_type start){	\
	size_type i;	\
	for(i=start;i<list->size;i++){	\
		if(cmp_func(list->buffer[i], e, NULL) == 0) return i;	\
	}	\
	return i;	\
}	\

#define define_list(name, e_type) define_list_core(name, e_type, u8i, 0xFFFFFU)

#define native_number_cmp(e1, e2, obj) (((e1) == (e2))? 0 : (((e1) < (e2))? -1 : 1))

#define define_native_list(name, e_type)	\
define_list_core(name, e_type, u8i, 0xFFFFFU);	\
define_list_ext(name, e_type, u8i, native_number_cmp);

define_native_list(u8list,  u1i);
define_native_list(u1v, u1i);
define_native_list(u16list, u2i);
define_native_list(u2v, u2i);
define_native_list(u32list, u4i);
define_native_list(u4v, u4i);
define_native_list(u64list, u8i);
define_native_list(u8v, u8i);

define_native_list(b8list,  b1i);
define_native_list(b1v, b1i);
define_native_list(b16list, b2i);
define_native_list(b2v, b2i);
define_native_list(b32list, b4i);
define_native_list(b4v, b4i);
define_native_list(b64list, b8i);
define_native_list(b8v, b8i);

define_native_list(f4v, f4i);
define_native_list(f8v, f8i);

define_list(vplist, void*);
define_list(cplist, char*);
// mem_share for deep dump of cplist
static inline size_t cplist_deep_obj_desc_cnt(void *list, int idx){
	if(idx == 0) return ((cplist*)list)->size;
	else return 1;
}
static const obj_desc_t cplist_deep_obj_desc = {.tag = "cplist_deep_dump", .size = sizeof(cplist), .n_child = 1, .mem_type = {3}, .addr = {offsetof(cplist, buffer)}, .desc = {(struct obj_desc_t*)&OBJ_DESC_CHAR_ARRAY}, .cnt = cplist_deep_obj_desc_cnt, .post=NULL};

#define define_recycle_list_array(name, list_type)	\
typedef struct {	\
	vplist *array;	\
	vplist *dustbin;	\
} name;	\
	\
static inline name* init_##name(){	\
	name *g;	\
	g = (name*)malloc(sizeof(name));	\
	g->buffer = init_vplist(4);	\
	g->dustbin = init_vplist(4);	\
	return g;	\
}	\
	\
static inline void free_##name(name *g){	\
	list_type *v;	\
	size_t i;	\
	for(i=0;i<g->array->size;i++){	\
		v = (list_type*)get_vplist(g->array, i);	\
		if(v) free_##list_type(v);	\
	}	\
	for(i=0;i<g->dustbin->size;i++){	\
		v = (list_type*)get_vplist(g->dustbin, i);	\
		if(v) free_##list_type(v);	\
	}	\
	free_vplist(g->array);	\
	free_vplist(g->dustbin);	\
	free(g);	\
}	\
	\
static inline list_type* fetch_##name(name *g){	\
	list_type *v;	\
	if(g->dustbin->size) v = (list_type*)g->dustbin->buffer[--g->dustbin->size];	\
	else v = init_##list_type(4);	\
	return v;	\
}	\
	\
static inline void recyc_##name(name *g, list_type *v){	\
	push_vplist(g->dustbin, v);	\
}	\
	\
static inline void recyc_all_##name(name *g, vplist *vs){	\
	append_vplist(g->dustbin, vs);	\
	vs->size = 0;	\
}

// e.g. define_recycle_list(u8r, u8i, u8i, *a = 0, NULL)
//       u8r, when increase an element, set the to 0 (*a = 0), when free it, do nothing (NULL)
#define define_recycle_list(list_type, e_type, size_type, e_init, e_free)	\
typedef struct {	\
	e_type *buffer;	\
	size_type size, cap;	\
	size_type *recyc;	\
	size_type rsize, rcap;	\
	u8i userdata;	\
} list_type;	\
	\
static inline list_type* init_##list_type(size_type size){	\
	list_type *list;	\
	if(size < 2) size = 2;	\
	list = malloc(sizeof(list_type));	\
	list->size = 0;	\
	list->cap  = size;	\
	list->buffer = calloc(size, sizeof(e_type));	\
	list->recyc = calloc(2, sizeof(size_type));	\
	list->rsize = 0;	\
	list->rcap  = 2;	\
	list->userdata = 0;	\
	return list;	\
}	\
	\
static inline void free_##list_type(list_type *list){	\
	e_type* a;	\
	size_type i;	\
	for(i=0;i<list->size;i++){	\
		a = list->buffer + i;	\
		UNUSED(e_free);	\
	}	\
	UNUSED(a);	\
	free(list->buffer);	\
	free(list->recyc);	\
	free(list);	\
}	\
	\
static inline void encap_##list_type(list_type *list, size_type inc){	\
	if(list->rsize >= inc) return;	\
	inc -= list->rsize;	\
	list->cap = encap_list((void**)&list->buffer, sizeof(e_type), list->size, list->cap, inc, 0, 0, 8);	\
}	\
	\
static inline size_type fetch_##list_type(list_type *list){	\
	e_type* a;	\
	if(list->rsize){	\
		return list->recyc[--list->rsize];	\
	}	\
	list->cap = encap_list((void**)&list->buffer, sizeof(e_type), list->size, list->cap, 1, 0, 0, 8);	\
	a = list->buffer + list->size;	\
	UNUSED(e_init);	\
	UNUSED(a);	\
	return list->size ++;	\
}	\
	\
static inline e_type* ref_##list_type(list_type *list, size_type idx){	\
	return list->buffer + idx;	\
}	\
	\
static inline size_type offset_##list_type(list_type *list, e_type *e){	\
	return e - list->buffer;	\
}	\
	\
static inline void recyc_##list_type(list_type *list, size_type idx){	\
	list->rcap = encap_list((void**)&list->recyc, sizeof(size_type), list->rsize, list->rcap, 1, 0, 0, 8);	\
	list->recyc[list->rsize++] = idx;	\
}	\
	\
static inline e_type* pop_##list_type(list_type *list){	\
	size_type idx;	\
	idx = fetch_##list_type(list);	\
	return ref_##list_type(list, idx);	\
}	\
	\
static inline void push_##list_type(list_type *list, e_type* e){	\
	recyc_##list_type(list, offset_##list_type(list, e));	\
}	\
	\
static inline void recyc_all_##list_type(list_type *list){	\
	size_type i;	\
	list->rsize = 0;	\
	list->rcap = encap_list((void**)&list->recyc, sizeof(size_type), list->rsize, list->rcap, list->size, 0, 0, 8);	\
	for(i=0;i<list->size;i++){	\
		list->recyc[i] = i;	\
	}	\
	list->rsize = list->size;	\
}

# endif
