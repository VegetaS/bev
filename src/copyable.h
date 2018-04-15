#ifndef BEV_SRC_COPYABLE_H
#define BEV_SRC_COPYABLE_H

namespace bev
{

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
};

};

#endif  // BEV_SRC_COPYABLE_H
