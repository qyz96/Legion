/* Copyright 2015 Stanford University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __LEGION_C_UTIL_H__
#define __LEGION_C_UTIL_H__

/**
 * \file legion_c_util.h
 * Legion C API: C++ Conversion Utilities
 */

#include "legion.h"
#include "legion_c.h"
#include "mapping_utilities.h"
#include "default_mapper.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace LegionRuntime {
  namespace HighLevel {
    class TaskResult {
    public:
      TaskResult(void) : value(NULL), value_size(0) {}

      TaskResult(const void *v, size_t v_size)
        : value(NULL), value_size(v_size) {
        if (!v) return;
        value = malloc(value_size);
        assert(value);
        memcpy(value, v, value_size);
      }

      TaskResult(const TaskResult &r) : value(NULL), value_size(r.value_size) {
        if (!r.value) return;
        value = malloc(value_size);
        assert(value);
        memcpy(value, r.value, value_size);
      }

      const TaskResult &operator=(const TaskResult &r) {
        if (value) free(value);

        value_size = r.value_size;
        if (!r.value) {
          value = NULL;
          return r;
        }
        value = malloc(value_size);
        assert(value);
        memcpy(value, r.value, value_size);
        return r;
      }

      ~TaskResult(void) {
        if (value) free(value);
      }

      size_t legion_buffer_size(void) const {
        return sizeof(size_t) + value_size;
      }
      size_t legion_serialize(void *buffer) const {
        *(size_t *)buffer = value_size;
        if (value) {
          memcpy((void *)(((size_t *)buffer)+1), value, value_size);
        }
        return sizeof(size_t) + value_size;
      }
      size_t legion_deserialize(const void *buffer) {
        value_size = *(const size_t *)buffer;
        value = malloc(value_size);
        assert(value);
        memcpy(value, (void *)(((const size_t *)buffer)+1), value_size);
        return sizeof(size_t) + value_size;
      }

   public:
      void *value;
      size_t value_size;
    };

    class CObjectWrapper {
    public:

      typedef Accessor::AccessorType::Generic Generic;
      typedef Accessor::AccessorType::SOA<0> SOA;
      typedef Accessor::RegionAccessor<Generic> AccessorGeneric;
      typedef Accessor::RegionAccessor<SOA, char> AccessorArray;

#define NEW_OPAQUE_WRAPPER(T_, T)                                       \
      static T_ wrap(T t) {                                             \
        T_ t_;                                                          \
        t_.impl = static_cast<void *>(t);                               \
        return t_;                                                      \
      }                                                                 \
      static const T_ wrap_const(const T t) {                           \
        T_ t_;                                                          \
        t_.impl = const_cast<void *>(static_cast<const void *>(t));     \
        return t_;                                                      \
      }                                                                 \
      static T unwrap(T_ t_) {                                          \
        return static_cast<T>(t_.impl);                                 \
      }                                                                 \
      static const T unwrap_const(const T_ t_) {                        \
        return static_cast<const T>(t_.impl);                           \
      }

      NEW_OPAQUE_WRAPPER(legion_runtime_t, Runtime *);
      NEW_OPAQUE_WRAPPER(legion_context_t, Context);
      NEW_OPAQUE_WRAPPER(legion_coloring_t, Coloring *);
      NEW_OPAQUE_WRAPPER(legion_domain_coloring_t, DomainColoring *);
      NEW_OPAQUE_WRAPPER(legion_index_space_allocator_t, IndexSpaceAllocator *);
      NEW_OPAQUE_WRAPPER(legion_argument_map_t, ArgumentMap *);
      NEW_OPAQUE_WRAPPER(legion_predicate_t, Predicate *);
      NEW_OPAQUE_WRAPPER(legion_phase_barrier_t, PhaseBarrier *);
      NEW_OPAQUE_WRAPPER(legion_future_t, Future *);
      NEW_OPAQUE_WRAPPER(legion_future_map_t, FutureMap *);
      NEW_OPAQUE_WRAPPER(legion_task_launcher_t, TaskLauncher *);
      NEW_OPAQUE_WRAPPER(legion_index_launcher_t, IndexLauncher *);
      NEW_OPAQUE_WRAPPER(legion_inline_launcher_t, InlineLauncher *);
      NEW_OPAQUE_WRAPPER(legion_copy_launcher_t, CopyLauncher *);
      NEW_OPAQUE_WRAPPER(legion_must_epoch_launcher_t, MustEpochLauncher *);
      NEW_OPAQUE_WRAPPER(legion_physical_region_t, PhysicalRegion *);
      NEW_OPAQUE_WRAPPER(legion_accessor_generic_t, AccessorGeneric *);
      NEW_OPAQUE_WRAPPER(legion_accessor_array_t, AccessorArray *);
      NEW_OPAQUE_WRAPPER(legion_index_iterator_t, IndexIterator *);
      NEW_OPAQUE_WRAPPER(legion_task_t, Task *);
      NEW_OPAQUE_WRAPPER(legion_inline_t, Inline *);
      NEW_OPAQUE_WRAPPER(legion_mappable_t, Mappable *);
      NEW_OPAQUE_WRAPPER(legion_region_requirement_t , RegionRequirement *);
      NEW_OPAQUE_WRAPPER(legion_machine_t, Machine *);
      NEW_OPAQUE_WRAPPER(legion_mapper_t, Mapper *);
      NEW_OPAQUE_WRAPPER(legion_machine_query_interface_t,
                         MappingUtilities::MachineQueryInterface*);
      NEW_OPAQUE_WRAPPER(legion_default_mapper_t, DefaultMapper*);
#undef NEW_OPAQUE_WRAPPER

      static legion_ptr_t
      wrap(ptr_t ptr)
      {
        legion_ptr_t ptr_;
        ptr_.value = ptr.value;
        return ptr_;
      }

      static ptr_t
      unwrap(legion_ptr_t ptr_)
      {
        ptr_t ptr;
        ptr.value = ptr_.value;
        return ptr;
      }

#define NEW_POINT_WRAPPER(T_, T, DIM)                \
      static T_ wrap(T t) {                          \
        T_ t_;                                       \
        std::copy(t.x, t.x + DIM, t_.x);             \
        return t_;                                   \
      }                                              \
      static T unwrap(T_ t_) {                       \
        T t(t_.x);                                   \
        return t;                                    \
      }

      NEW_POINT_WRAPPER(legion_point_1d_t, Point<1>, 1);
      NEW_POINT_WRAPPER(legion_point_2d_t, Point<2>, 2);
      NEW_POINT_WRAPPER(legion_point_3d_t, Point<3>, 3);
#undef NEW_POINT_WRAPPER

#define NEW_RECT_WRAPPER(T_, T)                         \
      static T_ wrap(T t) {                             \
        T_ t_;                                          \
        t_.lo = wrap(t.lo);                             \
        t_.hi = wrap(t.hi);                             \
        return t_;                                      \
      }                                                 \
      static T unwrap(T_ t_) {                          \
        T t(unwrap(t_.lo), unwrap(t_.hi));              \
        return t;                                       \
      }

      NEW_RECT_WRAPPER(legion_rect_1d_t, Rect<1>);
      NEW_RECT_WRAPPER(legion_rect_2d_t, Rect<2>);
      NEW_RECT_WRAPPER(legion_rect_3d_t, Rect<3>);
#undef NEW_RECT_WRAPPER

#define NEW_BLOCKIFY_WRAPPER(T_, T)                     \
      static T unwrap(T_ t_) {                          \
        T t(unwrap(t_.block_size));                     \
        return t;                                       \
      }

      NEW_BLOCKIFY_WRAPPER(legion_blockify_1d_t, Blockify<1>);
      NEW_BLOCKIFY_WRAPPER(legion_blockify_2d_t, Blockify<2>);
      NEW_BLOCKIFY_WRAPPER(legion_blockify_3d_t, Blockify<3>);
#undef NEW_RECT_WRAPPER
      static legion_domain_t
      wrap(Domain domain) {
        legion_domain_t domain_;
        domain_.is_id = domain.is_id;
        domain_.dim = domain.dim;
        std::copy(domain.rect_data, domain.rect_data + 2 * MAX_RECT_DIM, domain_.rect_data);
        return domain_;
      }

      static Domain
      unwrap(legion_domain_t domain_) {
        Domain domain;
        domain.is_id = domain_.is_id;
        domain.dim = domain_.dim;
        std::copy(domain_.rect_data, domain_.rect_data + 2 * MAX_RECT_DIM, domain.rect_data);
        return domain;
      }

      static legion_domain_point_t
      wrap(DomainPoint dp) {
        legion_domain_point_t dp_;
        dp_.dim = dp.dim;
        std::copy(dp.point_data, dp.point_data + MAX_POINT_DIM, dp_.point_data);
        return dp_;
      }

      static DomainPoint
      unwrap(legion_domain_point_t dp_) {
        DomainPoint dp;
        dp.dim = dp_.dim;
        std::copy(dp_.point_data, dp_.point_data + MAX_POINT_DIM, dp.point_data);
        return dp;
      }

      static legion_index_space_t
      wrap(IndexSpace is)
      {
        legion_index_space_t is_;
        is_.id = is.id;
        is_.tid = is.tid;
        return is_;
      }

      static IndexSpace
      unwrap(legion_index_space_t is_)
      {
        IndexSpace is;
        is.id = is_.id;
        is.tid = is_.tid;
        return is;
      }

      static legion_index_partition_t
      wrap(IndexPartition ip)
      {
        legion_index_partition_t ip_;
        ip_.id = ip.id;
        ip_.tid = ip.tid;
        return ip_;
      }

      static IndexPartition
      unwrap(legion_index_partition_t ip_)
      {
        IndexPartition ip;
        ip.id = ip_.id;
        ip.tid = ip_.tid;
        return ip;
      }

      static legion_index_allocator_t
      wrap(IndexAllocator allocator)
      {
        legion_index_allocator_t allocator_;
        allocator_.index_space = wrap(allocator.index_space);
        allocator_.allocator = wrap(allocator.allocator);
        return allocator_;
      }

      static IndexAllocator
      unwrap(legion_index_allocator_t allocator_)
      {
        IndexAllocator allocator(unwrap(allocator_.index_space),
                                 unwrap(allocator_.allocator));
        return allocator;
      } 

      static legion_field_space_t
      wrap(FieldSpace fs)
      {
        legion_field_space_t fs_;
        fs_.id = fs.id;
        return fs_;
      }

      static FieldSpace
      unwrap(legion_field_space_t fs_)
      {
        FieldSpace fs(fs_.id);
        return fs;
      }

      static legion_logical_region_t
      wrap(LogicalRegion r)
      {
        legion_logical_region_t r_;
        r_.tree_id = r.tree_id;
        r_.index_space = wrap(r.index_space);
        r_.field_space = wrap(r.field_space);
        return r_;
      }

      static LogicalRegion
      unwrap(legion_logical_region_t r_)
      {
        LogicalRegion r(r_.tree_id,
                        unwrap(r_.index_space),
                        unwrap(r_.field_space));
        return r;
      }

      static legion_logical_partition_t
      wrap(LogicalPartition r)
      {
        legion_logical_partition_t r_;
        r_.tree_id = r.tree_id;
        r_.index_partition = wrap(r.index_partition);
        r_.field_space = wrap(r.field_space);
        return r_;
      }

      static LogicalPartition
      unwrap(legion_logical_partition_t r_)
      {
        LogicalPartition r(r_.tree_id,
                           unwrap(r_.index_partition),
                           unwrap(r_.field_space));
        return r;
      }

      static legion_field_allocator_t
      wrap(FieldAllocator allocator)
      {
        legion_field_allocator_t allocator_;
        allocator_.field_space = wrap(allocator.field_space);
        allocator_.parent = wrap(allocator.parent);
        allocator_.runtime = wrap(allocator.runtime);
        return allocator_;
      }

      static FieldAllocator
      unwrap(legion_field_allocator_t allocator_)
      {
        FieldAllocator allocator(unwrap(allocator_.field_space),
                                 unwrap(allocator_.parent),
                                 unwrap(allocator_.runtime));
        return allocator;
      }

      static legion_task_argument_t
      wrap(TaskArgument arg)
      {
        legion_task_argument_t arg_;
        arg_.args = arg.get_ptr();
        arg_.arglen = arg.get_size();
        return arg_;
      }

      static TaskArgument
      unwrap(legion_task_argument_t arg_)
      {
        return TaskArgument(arg_.args, arg_.arglen);
      }

      static legion_task_result_t
      wrap(TaskResult result)
      {
        legion_task_result_t result_;
        result_.value = malloc(result.value_size);
        assert(result_.value);
        memcpy(result_.value, result.value, result.value_size);
        result_.value_size = result.value_size;
        return result_;
      }

      static TaskResult
      unwrap(legion_task_result_t result_)
      {
        return TaskResult(result_.value, result_.value_size);
      }

      static const legion_byte_offset_t
      wrap(const Accessor::ByteOffset offset)
      {
        legion_byte_offset_t offset_;
        offset_.offset = offset.offset;
        return offset_;
      }

      static const Accessor::ByteOffset
      unwrap(const legion_byte_offset_t offset_)
      {
        const Accessor::ByteOffset offset(offset_.offset);
        return offset;
      }

      static const legion_input_args_t
      wrap_const(const InputArgs arg)
      {
        legion_input_args_t arg_;
        arg_.argv = arg.argv;
        arg_.argc = arg.argc;
        return arg_;
      }

      static const InputArgs
      unwrap_const(const legion_input_args_t args_)
      {
        InputArgs args;
        args.argv = args_.argv;
        args.argc = args_.argc;
        return args;
      }

      static legion_task_config_options_t
      wrap(TaskConfigOptions options)
      {
        legion_task_config_options_t options_;
        options_.leaf = options.leaf;
        options_.inner = options.inner;
        options_.idempotent = options.idempotent;
        return options_;
      }

      static TaskConfigOptions
      unwrap(legion_task_config_options_t options_)
      {
        TaskConfigOptions options(options_.leaf,
                                  options_.inner,
                                  options_.idempotent);
        return options;
      }

      static legion_processor_t
      wrap(Processor proc)
      {
        legion_processor_t proc_;
        proc_.id = proc.id;
        return proc_;
      }

      static Processor
      unwrap(legion_processor_t proc_)
      {
        Processor proc;
        proc.id = proc_.id;
        return proc;
      }

      static legion_processor_kind_t
      wrap(Processor::Kind options)
      {
        return static_cast<legion_processor_kind_t>(options);
      }

      static Processor::Kind
      unwrap(legion_processor_kind_t options_)
      {
        return static_cast<Processor::Kind>(options_);
      }

      static legion_memory_t
      wrap(Memory mem)
      {
        legion_memory_t mem_;
        mem_.id = mem.id;
        return mem_;
      }

      static Memory
      unwrap(legion_memory_t mem_)
      {
        Memory mem;
        mem.id = mem_.id;
        return mem;
      }

      static legion_memory_kind_t
      wrap(Memory::Kind options)
      {
        return static_cast<legion_memory_kind_t>(options);
      }

      static Memory::Kind
      unwrap(legion_memory_kind_t options_)
      {
        return static_cast<Memory::Kind>(options_);
      }

      static legion_domain_split_t
      wrap(Mapper::DomainSplit domain_split) {
        legion_domain_split_t domain_split_;
        domain_split_.domain = wrap(domain_split.domain);
        domain_split_.proc = wrap(domain_split.proc);
        domain_split_.recurse = domain_split.recurse;
        domain_split_.stealable = domain_split.stealable;
        return domain_split_;
      }

      static Mapper::DomainSplit
      unwrap(legion_domain_split_t domain_split_) {
        Mapper::DomainSplit domain_split(
            unwrap(domain_split_.domain),
            unwrap(domain_split_.proc),
            domain_split_.recurse,
            domain_split_.stealable);
        return domain_split;
      }
    };
  }
}

#endif // __LEGION_C_UTIL_H__
