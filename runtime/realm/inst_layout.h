/* Copyright 2017 Stanford University, NVIDIA Corporation
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

// Layout descriptors for Realm RegionInstances

#ifndef REALM_INST_LAYOUT_H
#define REALM_INST_LAYOUT_H

#include "indexspace.h"

#include <vector>
#include <map>
#include <iostream>

namespace Realm {

  typedef int FieldID;

  class InstanceLayoutConstraints {
  public:
    InstanceLayoutConstraints(const std::vector<size_t>& field_sizes,
			      size_t block_size);
#ifdef REALM_USE_LEGION_LAYOUT_CONSTRAINTS
    InstanceLayoutConstraints(const Legion::LayoutConstraintSet& lcs);
#endif

    struct FieldInfo {
      FieldID field_id;
      int offset;
      int size;
      int alignment;
    };
    typedef std::vector<FieldInfo> FieldGroup;

    std::vector<FieldGroup> field_groups;
  };


  // instance layouts are templated on the type of the ZIndexSpace used to
  //  index them, but they all inherit from a generic version
  class InstanceLayoutGeneric {
  protected:
    // cannot be created directly
    InstanceLayoutGeneric(void);

  public:
    virtual ~InstanceLayoutGeneric(void);

    // adjusts offsets of all pieces by 'adjust_amt'
    virtual void relocate(size_t adjust_amt) = 0;

    virtual void print(std::ostream& os) const = 0;

    template <int N, typename T>
    static InstanceLayoutGeneric *choose_instance_layout(ZIndexSpace<N,T> is,
							 const InstanceLayoutConstraints& ilc);

    size_t bytes_used;
    size_t alignment_reqd;

    // we optimize for fields being laid out similarly, and have fields
    //  indirectly reference a piece list
    struct FieldLayout {
      int list_idx;
      int rel_offset;
    };

    std::map<FieldID, FieldLayout> fields;
  };

  std::ostream& operator<<(std::ostream& os, const InstanceLayoutGeneric& ilg);

  template <int N, typename T>
  class InstanceLayoutPiece {
  public:
    enum LayoutType {
      InvalidLayoutType,
      AffineLayoutType,
    };

    InstanceLayoutPiece(void);
    InstanceLayoutPiece(LayoutType _layout_type);
    virtual ~InstanceLayoutPiece(void);

    virtual void relocate(size_t base_offset) = 0;

    virtual void print(std::ostream& os) const = 0;

    LayoutType layout_type;
    ZRect<N,T> bounds;
  };

  template <int N, typename T>
  std::ostream& operator<<(std::ostream& os, const InstanceLayoutPiece<N,T>& ilp);

  template <int N, typename T>
  class AffineLayoutPiece : public InstanceLayoutPiece<N,T> {
  public:
    AffineLayoutPiece(void);

    virtual void relocate(size_t base_offset);

    virtual void print(std::ostream& os) const;

    ZPoint<N, size_t> strides;
    size_t offset;
  };

  template <int N, typename T>
  class InstancePieceList {
  public:
    InstancePieceList(void);
    ~InstancePieceList(void);

    const InstanceLayoutPiece<N,T> *find_piece(ZPoint<N,T> p) const;

    void relocate(size_t base_offset);

    std::vector<InstanceLayoutPiece<N,T> *> pieces;
    // placeholder for lookup structure (e.g. K-D tree)
  };

  template <int N, typename T>
  std::ostream& operator<<(std::ostream& os, const InstancePieceList<N,T>& ipl);

  template <int N, typename T>
  class InstanceLayout : public InstanceLayoutGeneric {
  public:
    InstanceLayout(void);
    virtual ~InstanceLayout(void);

    // adjusts offsets of pieces to start from 'base_offset'
    virtual void relocate(size_t base_offset);

    virtual void print(std::ostream& os) const;

    ZIndexSpace<N,T> space;
    std::vector<InstancePieceList<N,T> > piece_lists;
  };

}; // namespace Realm

#include "inst_layout.inl"

#endif // ifndef REALM_INST_LAYOUT_H


