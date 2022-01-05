#ifndef __ACCEXT_HPP
#define __ACCEXT_HPP

#include <boost/accumulators/framework/accumulator_base.hpp>
#include <boost/accumulators/framework/parameters/sample.hpp>
#include <boost/accumulators/statistics/max.hpp>

namespace boost {                           // Putting your accumulators in the
namespace accumulators {                    // impl namespace has some
namespace impl {                            // advantages. See below.

template<typename Sample>
struct first_accumulator                      // All accumulators should inherit from
  : accumulator_base                        // accumulator_base.
{
    typedef Sample result_type;             // The type returned by result() below.

    template<typename Args>                 // The constructor takes an argument pack.
    explicit first_accumulator(Args const & args)
      : seen_first(false)
    {
    }

    template<typename Args>                 // The accumulate function is the function
    void operator ()(Args const & args)     // call operator, and it also accepts an
    {
        if(!this->seen_first) {
            this->first = args[sample];
            this->seen_first = true;
        }
    }

    result_type result(dont_care) const     // The result function will also be passed
    {                                       // an argument pack, but we don't use it here,
        return this->first;                   // so we use "dont_care" as the argument type.
    }
private:
    Sample first;
    bool seen_first;
};

template<typename Sample>
struct last_accumulator                     // All accumulators should inherit from
  : accumulator_base                        // accumulator_base.
{
    typedef Sample result_type;             // The type returned by result() below.

    template<typename Args>                 // The constructor takes an argument pack.
    explicit last_accumulator(Args const & )
    {
    }

    template<typename Args>                 // The accumulate function is the function
    void operator ()(Args const & args)     // call operator, and it also accepts an
    {
        this->last = args[sample];
    }

    result_type result(dont_care) const     // The result function will also be passed
    {                                       // an argument pack, but we don't use it here,
        return this->last;                  // so we use "dont_care" as the argument type.
    }
private:
    Sample last;
};


}}}

namespace boost { namespace accumulators { namespace tag {

struct first : depends_on<>
{
    typedef accumulators::impl::first_accumulator< mpl::_1 > impl;
};

struct last : depends_on<>
{
    typedef accumulators::impl::last_accumulator< mpl::_1 > impl;
};


}}}


namespace boost {
namespace accumulators {                // By convention, we put extractors
namespace extract {                     // in the 'extract' namespace

    extractor< tag::first > const first = {}; // Simply define our extractor with
                                        // our feature tag, like this.
}
using extract::first;                    // Pull the extractor into the
                                        // enclosing namespace.
}}

namespace boost {
namespace accumulators {                // By convention, we put extractors
namespace extract {                     // in the 'extract' namespace

    extractor< tag::last > const last = {}; // Simply define our extractor with
                                        // our feature tag, like this.
}
using extract::last;                    // Pull the extractor into the
                                        // enclosing namespace.
}}

#endif
