#ifndef STL_LITE_H
#define STL_LITE_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace std_lite{


  /**
   *  @brief This does what you think it does.
   *  @param  a  A thing of arbitrary type.
   *  @param  b  Another thing of arbitrary type.
   *  @return   The lesser of the parameters.
   *
   *  This is the simple classic generic implementation.  It will work on
   *  temporary expressions, since they are only evaluated once, unlike a
   *  preprocessor macro.
  */
  template<typename _Tp>
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b)
    {
      //return __b < __a ? __b : __a;
      if (__b < __a)
	return __b;
      return __a;
    }


  /**
   *  @brief This does what you think it does.
   *  @param  a  A thing of arbitrary type.
   *  @param  b  Another thing of arbitrary type.
   *  @return   The greater of the parameters.
   *
   *  This is the simple classic generic implementation.  It will work on
   *  temporary expressions, since they are only evaluated once, unlike a
   *  preprocessor macro.
  */
  template<typename _Tp>
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b)
    {
      //return  __a < __b ? __b : __a;
      if (__a < __b)
	return __b;
      return __a;
    }


  /**
   *  @brief Swaps two values.
   *  @ingroup mutating_algorithms
   *  @param  __a  A thing of arbitrary type.
   *  @param  __b  Another thing of arbitrary type.
   *  @return   Nothing.
   */
  template<typename _Tp>
    inline void
      swap(_Tp& __a, _Tp& __b)
      {
          _Tp __tmp = __a;
          __a = __b;
          __b = __tmp;
      }


  /**
   *  This is one of the @link s20_3_1_base functor base classes@endlink.
   */
  template <class _Arg1, class _Arg2, class _Result>
    struct binary_function
    {
      typedef _Arg1 first_argument_type;   ///< the type of the first argument
                                           ///  (no surprises here)

      typedef _Arg2 second_argument_type;  ///< the type of the second argument
      typedef _Result result_type;         ///< type of the return type
    };


  /// pair holds two objects of arbitrary type.
  template<class _T1, class _T2>
    struct pair
    {
      typedef _T1 first_type;    ///<  @c first_type is the first bound type
      typedef _T2 second_type;   ///<  @c second_type is the second bound type

      _T1 first;                 ///< @c first is a copy of the first object
      _T2 second;                ///< @c second is a copy of the second object

      /** The default constructor creates @c first and @c second using their
       *  respective default constructors.  */
      pair()
      : first(), second() { }

      /** Two objects may be passed to a @c pair constructor to be copied.  */
      pair(const _T1& __a, const _T2& __b)
      : first(__a), second(__b) { }

      /** There is also a templated copy ctor for the @c pair class itself.  */
      template<class _U1, class _U2>
        pair(const pair<_U1, _U2>& __p)
	: first(__p.first), second(__p.second) { }
    };

  /// Two pairs of the same type are equal iff their members are equal.
  template<class _T1, class _T2>
    inline bool
    operator==(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __x.first == __y.first && __x.second == __y.second; }

  /// <http://gcc.gnu.org/onlinedocs/libstdc++/20_util/howto.html#pairlt>
  template<class _T1, class _T2>
    inline bool
    operator<(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __x.first < __y.first
	     || (!(__y.first < __x.first) && __x.second < __y.second); }

  /// Uses @c operator== to find the result.
  template<class _T1, class _T2>
    inline bool
    operator!=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__x == __y); }

  /// Uses @c operator< to find the result.
  template<class _T1, class _T2>
    inline bool
    operator>(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __y < __x; }

  /// Uses @c operator< to find the result.
  template<class _T1, class _T2>
    inline bool
    operator<=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__y < __x); }

  /// Uses @c operator< to find the result.
  template<class _T1, class _T2>
    inline bool
    operator>=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__x < __y); }

  /**
   *  @brief A convenience wrapper for creating a pair from two objects.
   *  @param  x  The first object.
   *  @param  y  The second object.
   *  @return   A newly-constructed pair<> object of the appropriate type.
   *
   *  The standard requires that the objects be passed by reference-to-const,
   *  but LWG issue #181 says they should be passed by const value.  We follow
   *  the LWG by default.
   */
  // _GLIBCXX_RESOLVE_LIB_DEFECTS
  // 181.  make_pair() unintended behavior
  template<class _T1, class _T2>
    inline pair<_T1, _T2>
    make_pair(_T1 __x, _T2 __y)
    { return pair<_T1, _T2>(__x, __y); }

  /**
   *  @brief Finds the first position in which @a val could be inserted
   *         without changing the ordering.
   *  @param  first   An iterator.
   *  @param  last    Another iterator.
   *  @param  val     The search term.
   *  @param  comp    A functor to use for comparisons.
   *  @return  An iterator pointing to the first element "not less than" @a val,
   *           or end() if every element is less than @a val.
   *  @ingroup binarysearch
   *
   *  The comparison function should have the same effects on ordering as
   *  the function used for the initial sort.
  */
  template<typename _ForwardIterator, typename _Tp, typename _Compare>
    _ForwardIterator
    lower_bound(_ForwardIterator __first, _ForwardIterator __last,
		const _Tp& __val, _Compare __comp)
    {
      typedef size_t _DistanceType;

      _DistanceType __len = __last - __first;
      _DistanceType __half;
      _ForwardIterator __middle;

      while (__len > 0)
	{
	  __half = __len >> 1;
	  __middle = __first;
	  __middle += __half;
	  if (__comp(*__middle, __val))
	    {
	      __first = __middle;
	      ++__first;
	      __len = __len - __half - 1;
	    }
	  else
	    __len = __half;
	}
      return __first;
    }

  /**
   *  @brief Finds the last position in which @a val could be inserted
   *         without changing the ordering.
   *  @param  first   An iterator.
   *  @param  last    Another iterator.
   *  @param  val     The search term.
   *  @param  comp    A functor to use for comparisons.
   *  @return  An iterator pointing to the first element greater than @a val,
   *           or end() if no elements are greater than @a val.
   *  @ingroup binarysearch
   *
   *  The comparison function should have the same effects on ordering as
   *  the function used for the initial sort.
  */
  template<typename _ForwardIterator, typename _Tp, typename _Compare>
    _ForwardIterator
    upper_bound(_ForwardIterator __first, _ForwardIterator __last,
		const _Tp& __val, _Compare __comp)
    {
      typedef size_t _DistanceType;
      _DistanceType __len = __last - __first;
      _DistanceType __half;
      _ForwardIterator __middle;

      while (__len > 0)
	{
	  __half = __len >> 1;
	  __middle = __first;
	  __middle += __half;
	  if (__comp(__val, *__middle))
	    __len = __half;
	  else
	    {
	      __first = __middle;
	      ++__first;
	      __len = __len - __half - 1;
	    }
	}
      return __first;
    }

  /**
   *  @brief Finds the largest subrange in which @a val could be inserted
   *         at any place in it without changing the ordering.
   *  @param  first   An iterator.
   *  @param  last    Another iterator.
   *  @param  val     The search term.
   *  @param  comp    A functor to use for comparisons.
   *  @return  An pair of iterators defining the subrange.
   *  @ingroup binarysearch
   *
   *  This is equivalent to
   *  @code
   *    std_lite::make_pair(lower_bound(first, last, val, comp),
   *                        upper_bound(first, last, val, comp))
   *  @endcode
   *  but does not actually call those functions.
  */
  template<typename _ForwardIterator, typename _Tp, typename _Compare>
    pair<_ForwardIterator, _ForwardIterator>
    equal_range(_ForwardIterator __first, _ForwardIterator __last,
		const _Tp& __val,
		_Compare __comp)
    {

      typedef size_t _DistanceType;

      _DistanceType __len = __last - __first;
      _DistanceType __half;
      _ForwardIterator __middle, __left, __right;

      while (__len > 0)
	{
	  __half = __len >> 1;
	  __middle = __first;
	  __middle += __half;
	  if (__comp(*__middle, __val))
	    {
	      __first = __middle;
	      ++__first;
	      __len = __len - __half - 1;
	    }
	  else if (__comp(__val, *__middle))
	    __len = __half;
	  else
	    {
	      __left = lower_bound(__first, __middle, __val, __comp);
	      __first += __len;
	      __right = upper_bound(++__middle, __first, __val, __comp);
	      return pair<_ForwardIterator, _ForwardIterator>(__left, __right);
	    }
	}
      return pair<_ForwardIterator, _ForwardIterator>(__first, __first);
    }


}
#endif
