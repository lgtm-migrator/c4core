#ifndef _C4_SUBSTR_HPP_
#define _C4_SUBSTR_HPP_

#include <string.h>
#include <type_traits>

#include "./config.hpp"
#include "./error.hpp"

C4_BEGIN_NAMESPACE(c4)

template< class C > class basic_substring;

/**
 * @see to_substr
 * @see to_csubstr
 * */
using substr = basic_substring< char >;
/** @see to_csubstr */
using csubstr = basic_substring< const char >;

template< class OStream, class C >
inline OStream& operator<< (OStream& s, basic_substring< C > sp)
{
    s.write(sp.str, sp.len);
    return s;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename C>
static inline void _do_reverse(C *C4_RESTRICT first, C *C4_RESTRICT last)
{
    while(last > first)
    {
        C tmp = *last;
        *last-- = *first;
        *first++ = tmp;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a span of characters and a length. Works like a writeable string_view. */
template< class C >
class basic_substring
{

// https://stackoverflow.com/questions/43051882/how-to-disable-a-class-member-funrtion-for-certain-template-types
#define C4_DISABLED_FOR_CONST_C(ret_type) \
    template <typename U=C> \
    typename std::enable_if<std::is_const<U>::value == false, ret_type>::type
// non-const-to-const
#define C4_NC2C(tt) \
    typename std::enable_if<std::is_const<C>::value && ( ! std::is_const<tt>::value), tt>::type

public:

    C *str;
    size_t len;

public:

    using  CC = typename std::add_const< C >::type;
    using NCC = typename std::remove_const< C >::type;

    using basic_csubstr = basic_substring<  CC >;
    using basic_ncsubstr = basic_substring< NCC >;

    using char_type = C;
    using size_type = size_t;

    using iterator = C*;
    using const_iterator = CC*;

    enum : size_t { npos = (size_t)-1, NONE = (size_t)-1 };

public:

    /// convert automatically to substring of const C
    operator basic_csubstr () const { basic_csubstr s(str, len); return s; }

public:

    basic_substring() : str(nullptr), len(0) {}

    basic_substring(basic_substring const&) = default;
    basic_substring(basic_substring     &&) = default;

    basic_substring& operator= (basic_substring const&) = default;
    basic_substring& operator= (basic_substring     &&) = default;

public:

    //basic_substring(C *s_) : str(s_), len(s_ ? strlen(s_) : 0) {}
    /** the overload for receiving a single C* pointer will always
     * hide the array[N] overload. So it is disabled. If you want to
     * construct a substr from a single pointer containing a C-style string,
     * you can call c4::yml::to_substr()/c4::yml::to_csubstr().
     * @see c4::yml::to_substr()
     * @see c4::yml::to_csubstr() */
    template< size_t N >
    basic_substring(C (&s_)[N]) : str(s_), len(N-1) {}
    basic_substring(C *s_, size_t len_) : str(s_), len(len_) { C4_ASSERT(str || !len_); }
    basic_substring(C *beg_, C *end_) : str(beg_), len(end_ - beg_) { C4_ASSERT(end_ >= beg_); }

	//basic_span& operator= (C *s_) { this->assign(s_); return *this; }
	template< size_t N >
	basic_substring& operator= (C (&s_)[N]) { this->assign<N>(s_); return *this; }

    //void assign(C *s_) { str = (s_); len = (s_ ? strlen(s_) : 0); }
    /** the overload for receiving a single C* pointer will always
     * hide the array[N] overload. So it is disabled. If you want to
     * construct a substr from a single pointer containing a C-style string,
     * you can call c4::yml::to_substr()/c4::yml::to_csubstr().
     * @see c4::yml::to_substr()
     * @see c4::yml::to_csubstr() */
    template< size_t N >
    void assign(C (&s_)[N]) { str = (s_); len = (N-1); }
    void assign(C *s_, size_t len_) { str = s_; len = len_; C4_ASSERT(str || !len_); }
    void assign(C *beg_, C *end_) { C4_ASSERT(end_ >= beg_); str = (beg_); len = (end_ - beg_); }

public:

    // when the char type is const, allow construction and assignment from non-const chars

    template< size_t N, class U=NCC > explicit basic_substring(C4_NC2C(U) (&s_)[N]) { str = s_; len = N-1; }
    template<           class U=NCC >          basic_substring(C4_NC2C(U) *s_, size_t len_) { str = s_; len = len_; }
    template<           class U=NCC >          basic_substring(C4_NC2C(U) *beg_, C4_NC2C(U) *end_) { C4_ASSERT(end_ >= beg_); str = beg_; len = end_ - beg_;  }

    template< size_t N, class U=NCC > void assign(C4_NC2C(U) (&s_)[N]) { str = s_; len = N-1; }
    template<           class U=NCC > void assign(C4_NC2C(U) *s_, size_t len_) { str = s_; len = len_; }
    template<           class U=NCC > void assign(C4_NC2C(U) *beg_, C4_NC2C(U) *end_) { C4_ASSERT(end_ >= beg_); str = beg_; len = end_ - beg_;  }

    template< size_t N, class U=NCC >
    basic_substring& operator=(C4_NC2C(U) (&s_)[N]) { str = s_; len = N-1; return *this; }

public:

    void clear() { str = nullptr; len = 0; }

    bool has_str() const { return ! empty() && str[0] != C(0); }
    bool empty() const { return (len == 0 || str == nullptr); }
    bool not_empty() const { return (len != 0 && str != nullptr); }
    size_t size() const { return len; }

    iterator begin() { return str; }
    iterator end  () { return str + len; }

    const_iterator begin() const { return str; }
    const_iterator end  () const { return str + len; }

    C      * data()       { return str; }
    C const* data() const { return str; }

    inline C      & operator[] (size_t i)       { C4_ASSERT(i >= 0 && i < len); return str[i]; }
    inline C const& operator[] (size_t i) const { C4_ASSERT(i >= 0 && i < len); return str[i]; }

public:

    int compare(basic_csubstr const that) const
    {
        size_t n = len < that.len ? len : that.len;
        int ret = strncmp(str, that.str, n);
        if(ret == 0 && len != that.len)
        {
            ret = len < that.len ? -1 : 1;
        }
        return ret;
    }

    int compare(C const c) const
    {
        return compare(basic_csubstr(&c, 1));
    }

    inline bool operator== (basic_csubstr const that) const { return this->compare(that) == 0; }
    inline bool operator!= (basic_csubstr const that) const { return this->compare(that) != 0; }
    inline bool operator<  (basic_csubstr const that) const { return this->compare(that) <  0; }
    inline bool operator>  (basic_csubstr const that) const { return this->compare(that) >  0; }
    inline bool operator<= (basic_csubstr const that) const { return this->compare(that) <= 0; }
    inline bool operator>= (basic_csubstr const that) const { return this->compare(that) >= 0; }

public:

    /** true if *this is a sub of that */
    inline bool is_contained(basic_csubstr const super) const
    {
        return begin() >= super.begin() && end() <= super.end();
    }

    /** true if that is a sub of this */
    inline bool contains(basic_csubstr const sub) const
    {
        return sub.begin() >= begin() && sub.end() <= end();
    }

public:

    /** return [first,first+num[ */
    basic_substring sub(size_t first, size_t num=npos) const
    {
        size_t rnum = num != npos ? num : len - first;
        C4_ASSERT((first >= 0 && first + rnum <= len) || num == 0);
        return basic_substring(str + first, rnum);
    }

    /** return [first,last[ */
    basic_substring range(size_t first, size_t last=npos) const
    {
        last = last != npos ? last : len;
        C4_ASSERT(first <= last);
        C4_ASSERT(first >= 0 && first <= len);
        C4_ASSERT(last  >= 0 && last  <= len);
        return basic_substring(str + first, last - first);
    }

    /** return [0,num[*/
    basic_substring first(size_t num) const
    {
        return sub(0, num);
    }

    /** return [len-num,len[*/
    basic_substring last(size_t num) const
    {
        C4_ASSERT(num <= len);
        return sub(len - num);
    }

public:

    basic_substring left_of(size_t pos, bool include_pos = false) const
    {
        if(pos == npos) return *this;
        if( ! include_pos && pos > 0) --pos;
        return sub(0, pos+1/* bump because this arg is a size, not a pos*/);
    }

    basic_substring right_of(size_t pos, bool include_pos = false) const
    {
        if(pos == npos) return sub(0, 0);
        if( ! include_pos) ++pos;
        return sub(pos);
    }

public:

    basic_substring left_of(basic_csubstr const ss) const
    {
        auto ssb = ss.begin();
        auto b = begin();
        auto e = end();
        if(ssb >= b && ssb <= e)
        {
            return sub(0, ssb - b);
        }
        else
        {
            return sub(0, 0);
        }
    }

    basic_substring right_of(basic_csubstr const ss) const
    {
        auto sse = ss.end();
        auto b = begin();
        auto e = end();
        if(sse >= b && sse <= e)
        {
            return sub(sse - b, e - sse);
        }
        else
        {
            return sub(0, 0);
        }
    }

public:

    /** trim left */
    basic_substring triml(const C c) const
    {
        //return right_of(first_not_of(c), /*include_pos*/true);
        return triml({&c, 1});
    }
    /** trim left ANY of the characters */
    basic_substring triml(basic_csubstr chars) const
    {
        //return right_of(first_not_of(chars), /*include_pos*/true);
        if( ! empty())
        {
            size_t pos = first_not_of(chars, 0);
            if(pos != npos)
            {
                return sub(pos);
            }
        }
        return sub(0, 0);
    }

    /** trim right */
    basic_substring trimr(const C c) const
    {
        //return left_of(last_not_of(c), /*include_pos*/true);
        return trimr({&c, 1});
    }
    /** trim right ANY of the characters */
    basic_substring trimr(basic_csubstr chars) const
    {
        //return left_of(last_not_of(chars), /*include_pos*/true);
        if( ! empty())
        {
            size_t pos = last_not_of(chars, npos);
            if(pos != npos)
            {
                return sub(0, pos+1);
            }
        }
        return sub(0, 0);
    }

    /** trim left and right */
    basic_substring trim(const C c) const
    {
        return triml(c).trimr(c);
    }
    /** trim left and right ANY of the characters */
    basic_substring trim(basic_csubstr const chars) const
    {
        return triml(chars).trimr(chars);
    }

public:

    basic_substring stripl(basic_csubstr s) const
    {
        if( ! begins_with(s)) return *this;
        return sub(s.len < len ? s.len : len);
    }

    basic_substring stripr(basic_csubstr s) const
    {
        if( ! ends_with(s)) return *this;
        return left_of(len - (s.len < len ? s.len : len));
    }

public:

    inline size_t find(const C c, size_t start_pos=0) const
    {
        return first_of(c, start_pos);
    }
    inline size_t find(basic_csubstr chars, size_t start_pos=0) const
    {
        C4_ASSERT(start_pos == npos || (start_pos >= 0 && start_pos <= len));
        if(len < chars.len) return npos;
        for(size_t i = start_pos, e = len - chars.len + 1; i < e; ++i)
        {
            bool gotit = true;
            for(size_t j = 0; j < chars.len; ++j)
            {
                C4_ASSERT(i + j < len);
                if(str[i + j] != chars.str[j])
                {
                    gotit = false;
                    break;
                }
            }
            if(gotit)
            {
                return i;
            }
        }
        return npos;
    }

public:

    struct first_of_any_result
    {
        size_t which;
        size_t pos;
        inline operator bool() const { return which != NONE && pos != npos; }
    };

    first_of_any_result first_of_any(basic_csubstr s0, basic_csubstr s1) const
    {
        basic_csubstr s[2] = {s0, s1};
        return first_of_any_iter(&s[0], &s[0] + 2);
    }

    first_of_any_result first_of_any(basic_csubstr s0, basic_csubstr s1, basic_csubstr s2) const
    {
        basic_csubstr s[3] = {s0, s1, s2};
        return first_of_any_iter(&s[0], &s[0] + 3);
    }

    first_of_any_result first_of_any(basic_csubstr s0, basic_csubstr s1, basic_csubstr s2, basic_csubstr s3) const
    {
        basic_csubstr s[4] = {s0, s1, s2, s3};
        return first_of_any_iter(&s[0], &s[0] + 4);
    }

    first_of_any_result first_of_any(basic_csubstr s0, basic_csubstr s1, basic_csubstr s2, basic_csubstr s3, basic_csubstr s4) const
    {
        basic_csubstr s[5] = {s0, s1, s2, s3, s4};
        return first_of_any_iter(&s[0], &s[0] + 5);
    }

    template< class It >
    first_of_any_result first_of_any_iter(It first_span, It last_span) const
    {
        for(size_t i = 0; i < len; ++i)
        {
            size_t curr = 0;
            for(It it = first_span; it != last_span; ++curr, ++it)
            {
                auto const& chars = *it;
                if((i + chars.len) > len) continue;
                bool gotit = true;
                for(size_t j = 0; j < chars.len; ++j)
                {
                    C4_ASSERT(i + j < len);
                    if(str[i + j] != chars[j])
                    {
                        gotit = false;
                        break;
                    }
                }
                if(gotit)
                {
                    return {curr, i};
                }
            }
        }
        return {NONE, npos};
    }

public:

    inline bool begins_with(const C c) const
    {
        return len > 0 ? str[0] == c : false;
    }
    inline bool begins_with(const C c, size_t num) const
    {
        if(len < num) return false;
        for(size_t i = 0; i < num; ++i)
        {
            if(str[i] != c) return false;
        }
        return true;
    }
    inline bool begins_with(basic_csubstr pattern) const
    {
        if(len < pattern.len) return false;
        for(size_t i = 0; i < pattern.len; ++i)
        {
            if(str[i] != pattern[i]) return false;
        }
        return true;
    }
    inline bool begins_with_any(basic_csubstr pattern) const
    {
        return first_of(pattern) == 0;
    }

    inline bool ends_with(const C c) const
    {
        return len > 0 ? str[len-1] == c : false;
    }
    inline bool ends_with(const C c, size_t num) const
    {
        if(len < num) return false;
        for(size_t i = len - num; i < len; ++i)
        {
            if(str[i] != c) return false;
        }
        return true;
    }
    inline bool ends_with(basic_csubstr pattern) const
    {
        if(len < pattern.len) return false;
        for(size_t i = 0, s = len-pattern.len; i < pattern.len; ++i)
        {
            if(str[s+i] != pattern[i]) return false;
        }
        return true;
    }
    inline bool ends_with_any(basic_csubstr chars) const
    {
        if(len == 0) return false;
        return last_of(chars) == len - 1;
    }

public:

    size_t first_of(const C c, size_t start=0) const
    {
        C4_ASSERT(start == npos || (start >= 0 && start <= len));
        for(size_t i = start; i < len; ++i)
        {
            if(str[i] == c) return i;
        }
        return npos;
    }

    size_t last_of(const C c, size_t start=npos) const
    {
        C4_ASSERT(start == npos || (start >= 0 && start <= len));
        if(start == npos) start = len;
        for(size_t i = start-1; i != size_t(-1); --i)
        {
            if(str[i] == c) return i;
        }
        return npos;
    }

    size_t first_of(basic_csubstr chars, size_t start=0) const
    {
        C4_ASSERT(start == npos || (start >= 0 && start <= len));
        for(size_t i = start; i < len; ++i)
        {
            for(size_t j = 0; j < chars.len; ++j)
            {
                if(str[i] == chars[j]) return i;
            }
        }
        return npos;
    }

    size_t last_of(basic_csubstr chars, size_t start=npos) const
    {
        C4_ASSERT(start == npos || (start >= 0 && start <= len));
        if(start == npos) start = len;
        for(size_t i = start-1; i != size_t(-1); --i)
        {
            for(size_t j = 0; j < chars.len; ++j)
            {
                if(str[i] == chars[j]) return i;
            }
        }
        return npos;
    }

public:

    size_t first_not_of(const C c, size_t start=0) const
    {
        C4_ASSERT((start >= 0 && start < len) || (start == len && len == 0));
        for(size_t i = start; i < len; ++i)
        {
            if(str[i] != c) return i;
        }
        return npos;
    }

    size_t last_not_of(const C c, size_t start=npos) const
    {
        C4_ASSERT(start == npos || (start >= 0 && start <= len));
        if(start == npos) start = len;
        for(size_t i = start-1; i != size_t(-1); --i)
        {
            if(str[i] != c) return i;
        }
        return npos;
    }

    size_t first_not_of(basic_csubstr chars, size_t start=0) const
    {
        C4_ASSERT((start >= 0 && start < len) || (start == len && len == 0));
        for(size_t i = start; i < len; ++i)
        {
            bool gotit = true;
            for(size_t j = 0; j < chars.len; ++j)
            {
                if(str[i] == chars.str[j])
                {
                    gotit = false;
                    break;
                }
            }
            if(gotit)
            {
                return i;
            }
        }
        return npos;
    }

    size_t last_not_of(basic_csubstr chars, size_t start=npos) const
    {
        C4_ASSERT(start == npos || (start >= 0 && start <= len));
        if(start == npos) start = len;
        for(size_t i = len-1; i != size_t(-1); --i)
        {
            bool gotit = true;
            for(size_t j = 0; j < chars.len; ++j)
            {
                if(str[i] == chars.str[j])
                {
                    gotit = false;
                    break;
                }
            }
            if(gotit)
            {
                return i;
            }
        }
        return npos;
    }

public:

    /** get the range delimited by a single open-close character (eg, quotes).
     * The open-close character can be escaped. */
    basic_substring pair_range_esc(CC open_close, CC escape=CC('\\'))
    {
        size_t b = find(open_close);
        if(b == npos) return basic_substring();
        size_t nest_level = 0;
        for(size_t i = b+1; i < len; ++i)
        {
            CC c = str[i];
            if(c == open_close)
            {
                if(str[i-1] != escape)
                {
                    return range(b, i+1);
                }
            }
        }
        return basic_substring();
    }

    /** get the range delimited by an open-close pair of characters.
     * There must be no nested pairs.
     * No checks for escapes are performed. */
    basic_substring pair_range(CC open, CC close) const
    {
        size_t b = find(open);
        if(b == npos) return basic_substring();
        size_t e = find(close, b+1);
        if(e == npos) return basic_substring();
        basic_substring ret = range(b, e+1);
        C4_ASSERT(ret.sub(1).find(open) == npos);
        return ret;
    }

    /** get the range delimited by an open-close pair of characters,
     * with possibly nested occurrences. No checks for escapes are
     * performed. */
    basic_substring pair_range_nested(CC open, CC close) const
    {
        size_t b = find(open);
        if(b == npos) return basic_substring();
        size_t e, curr = b+1, count = 0;
        const char both[] = {open, close, '\0'};
        while((e = first_of(both, curr)) != npos)
        {
            if(str[e] == open)
            {
                ++count;
                curr = e+1;
            }
            else if(str[e] == close)
            {
                if(count == 0) return range(b, e+1);
                --count;
                curr = e+1;
            }
        }
        return basic_substring();
    }

public:

    /** get the first span consisting exclusively of non-empty characters */
    basic_substring first_non_empty_span() const
    {
        basic_csubstr empty_chars(" \n\r\t");
        size_t pos = first_not_of(empty_chars);
        if(pos == npos) return sub(0, 0);
        auto ret = sub(pos);
        pos = ret.first_of(empty_chars);
        return ret.sub(0, pos);
    }

    /** get the first span which can be interpreted as an unsigned integer */
    basic_substring first_uint_span() const
    {
        basic_substring ne = first_non_empty_span();
        if(ne.first_of_any("0x", "0X"))
        {
            if(ne.len == 2) return basic_substring();
            for(size_t i = 2; i < ne.len; ++i)
            {
                char c = ne.str[i];
                if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                    continue;
                else
                {
                    return ne.sub(0, i);
                }
            }
        }
        else
        {
            for(size_t i = 0; i < ne.len; ++i)
            {
                char c = ne.str[i];
                if(c < '0' || c > '9')
                {
                    return ne.sub(0, i);
                }
            }
        }
        return ne;
    }

    /** get the first span which can be interpreted as a signed integer */
    basic_substring first_int_span() const
    {
        basic_substring ne = first_non_empty_span();
        if(ne.first_of_any("-0x", "-0X", "0x", "0X"))
        {
            if(ne.len == 2) return basic_substring();
            for(size_t i = (2 + (ne[0] == '-')); i < ne.len; ++i)
            {
                char c = ne.str[i];
                if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                    continue;
                else
                {
                    return ne.sub(0, i);
                }
            }
        }
        else
        {
            for(size_t i = 0; i < ne.len; ++i)
            {
                char c = ne.str[i];
                if(c == '-')
                {
                    if(i != 0)
                    {
                        return ne.sub(0, i);
                    }
                }
                else if(c < '0' || c > '9')
                {
                    return ne.sub(0, i);
                }
            }
        }
        return ne;
    }

    /** get the first span which can be interpreted as a real (floating-point) number */
    basic_substring first_real_span() const
    {
        basic_substring ne = first_non_empty_span();
        for(size_t i = 0; i < ne.len; ++i)
        {
            char c = ne.str[i];
            if(c == '-' || c == '+')
            {
                if(i == 0) // a leading signal is valid
                {
                    continue;
                }
                else // we can also have a sign for the exponent
                {
                    char e = ne[i-1];
                    if(e == 'e' || e == 'E')
                    {
                        continue;
                    }
                    else
                    {
                        return ne.sub(0, i);
                    }
                }
            }
            else if((c < '0' || c > '9') && (c != '.' && c != 'e' && c != 'E'))
            {
                return ne.sub(0, i);
            }
        }
        return ne;
    }

public:

    /** returns true if the string has not been exhausted yet, meaning
     * it's ok to call next_split() again. When no instance of sep
     * exists in the string, returns the full string. When the input
     * is an empty string, the output string is the empty string. */
    bool next_split(C sep, size_t *C4_RESTRICT start_pos, basic_substring *C4_RESTRICT out) const
    {
        if(C4_LIKELY(*start_pos < len))
        {
            for(size_t i = *start_pos, e = len; i < e; i++)
            {
                if(str[i] == sep)
                {
                    out->assign(str + *start_pos, i - *start_pos);
                    *start_pos = i+1;
                    return true;
                }
            }
            out->assign(str + *start_pos, len - *start_pos);
            *start_pos = len + 1;
            return true;
        }
        else
        {
            bool valid = len > 0 && (*start_pos == len);
            if(valid && !empty() && str[len-1] == sep)
            {
                out->assign(str + len, (size_t)0); // the cast is needed to prevent overload ambiguity
            }
            else
            {
                out->assign(str + len + 1, (size_t)0); // the cast is needed to prevent overload ambiguity
            }
            *start_pos = len + 1;
            return valid;
        }
    }

private:

    struct split_proxy_impl
    {
        struct split_iterator_impl
        {
            split_proxy_impl const* m_proxy;
            basic_substring m_str;
            size_t m_pos;
            NCC m_sep;

            split_iterator_impl(split_proxy_impl const* proxy, size_t pos, C sep)
                : m_proxy(proxy), m_pos(pos), m_sep(sep)
            {
                _tick();
            }

            void _tick()
            {
                m_proxy->m_str.next_split(m_sep, &m_pos, &m_str);
            }

            split_iterator_impl& operator++ () { _tick(); return *this; }
            split_iterator_impl  operator++ (int) { split_iterator_impl it = *this; _tick(); return it; }

            basic_substring& operator*  () { return  m_str; }
            basic_substring* operator-> () { return &m_str; }

            bool operator!= (split_iterator_impl const& that) const
            {
                return !(this->operator==(that));
            }
            bool operator== (split_iterator_impl const& that) const
            {
                C4_XASSERT((m_sep == that.m_sep) && "cannot compare split iterators with different separators");
                if(m_str.size() != that.m_str.size()) return false;
                if(m_str.data() != that.m_str.data()) return false;
                return m_pos == that.m_pos;
            }
        };

        basic_substring m_str;
        size_t m_start_pos;
        C m_sep;

        split_proxy_impl(basic_substring str, size_t start_pos, C sep)
            : m_str(str), m_start_pos(start_pos), m_sep(sep)
        {
        }

        split_iterator_impl begin() const
        {
            auto it = split_iterator_impl(this, m_start_pos, m_sep);
            return it;
        }
        split_iterator_impl end() const
        {
            size_t pos = m_str.size() + 1;
            auto it = split_iterator_impl(this, pos, m_sep);
            return it;
        }
    };

public:

    using split_proxy = split_proxy_impl;

    split_proxy split(C sep, size_t start_pos=0) const
    {
        C4_XASSERT((start_pos >= 0 && start_pos < len) || empty());
        auto ss = sub(0, len);
        auto it = split_proxy(ss, start_pos, sep);
        return it;
    }

public:

    basic_substring basename(C sep=C('/')) const
    {
        auto ss = pop_right(sep, /*skip_empty*/true);
        ss = ss.trimr(sep);
        return ss;
    }

    basic_substring dirname(C sep=C('/')) const
    {
        auto ss = basename(sep);
        ss = ss.empty() ? *this : left_of(ss);
        return ss;
    }

public:

    /** pop right: return the first split from the right. Use
     * gpop_left() to get the reciprocal part.
     * pop_right() can be used to retrieve the extension of a filename:
     * @begincode
     *   auto ext = s.pop_right('.');
     * @endcode
     */
    basic_substring pop_right(C sep=C('/'), bool skip_empty=false) const
    {
        if(C4_LIKELY(len > 1))
        {
            auto pos = last_of(sep);
            if(pos != npos)
            {
                if(pos + 1 < len) // does not end with sep
                {
                    return sub(pos + 1); // return from sep to end
                }
                else // the string ends with sep
                {
                    if( ! skip_empty)
                    {
                        return sub(pos + 1, 0);
                    }
                    auto ppos = last_not_of(sep); // skip repeated seps
                    if(ppos == npos) // the string is all made of seps
                    {
                        return sub(0, 0);
                    }
                    // find the previous sep
                    auto pos0 = last_of(sep, ppos);
                    if(pos0 == npos) // only the last sep exists
                    {
                        return sub(0); // return the full string (because skip_empty is true)
                    }
                    ++pos0;
                    return sub(pos0);
                }
            }
            else // no sep was found, return the full string
            {
                return *this;
            }
        }
        else if(len == 1)
        {
            if(begins_with(sep))
            {
                return sub(0, 0);
            }
            return *this;
        }
        else // an empty string
        {
            return basic_substring();
        }
    }

    /** return the first split from the left. Use gpop_right() to get
     * the reciprocal part. */
    basic_substring pop_left(C sep = C('/'), bool skip_empty=false) const
    {
        if(C4_LIKELY(len > 1))
        {
            auto pos = first_of(sep);
            if(pos != npos)
            {
                if(pos > 0)  // does not start with sep
                {
                    return sub(0, pos); //  return everything up to it
                }
                else  // the string starts with sep
                {
                    if( ! skip_empty)
                    {
                        return sub(0, 0);
                    }
                    auto ppos = first_not_of(sep); // skip repeated seps
                    if(ppos == npos) // the string is all made of seps
                    {
                        return sub(0, 0);
                    }
                    // find the next sep
                    auto pos0 = first_of(sep, ppos);
                    if(pos0 == npos) // only the first sep exists
                    {
                        return sub(0); // return the full string (because skip_empty is true)
                    }
                    C4_XASSERT(pos0 > 0);
                    // return everything up to the second sep
                    return sub(0, pos0);
                }
            }
            else // no sep was found, return the full string
            {
                return sub(0);
            }
        }
        else if(len == 1)
        {
            if(begins_with(sep))
            {
                return sub(0, 0);
            }
            return sub(0);
        }
        else // an empty string
        {
            return basic_substring();
        }
    }

public:

    basic_substring gpop_left(C sep = C('/'), bool skip_empty=false) const
    {
        auto ss = pop_right(sep, skip_empty);
        ss = left_of(ss);
        if(ss.find(sep) != npos)
        {
            if(ss.ends_with(sep))
            {
                if(skip_empty)
                {
                    ss = ss.trimr(sep);
                }
                else
                {
                    ss = ss.sub(0, ss.len-1); // safe to subtract because ends_with(sep) is true
                }
            }
        }
        return ss;
    }

    basic_substring gpop_right(C sep = C('/'), bool skip_empty=false) const
    {
        auto ss = pop_left(sep, skip_empty);
        ss = right_of(ss);
        if(ss.find(sep) != npos)
        {
            if(ss.begins_with(sep))
            {
                if(skip_empty)
                {
                    ss = ss.triml(sep);
                }
                else
                {
                    ss = ss.sub(1);
                }
            }
        }
        return ss;
    }

public:

    C4_DISABLED_FOR_CONST_C(void) copy_from(basic_csubstr that, size_t ifirst=0, size_t num=npos)
    {
        C4_ASSERT(ifirst >= 0 && ifirst <= len);
        num = num != npos ? num : len - ifirst;
        num = num < that.len ? num : that.len;
        memcpy(str + sizeof(C) * ifirst, that.str, sizeof(C) * num);
    }

public:

    C4_DISABLED_FOR_CONST_C(void) reverse()
    {
        if(len == 0) return;
        _do_reverse(str, str + len - 1);
    }

    C4_DISABLED_FOR_CONST_C(void) reverse_sub(size_t ifirst, size_t num)
    {
        C4_ASSERT(ifirst >= 0 && ifirst <= len);
        C4_ASSERT(ifirst + num >= 0 && ifirst + num <= len);
        if(num == 0) return;
        _do_reverse(str + ifirst, ifirst + num - 1);
    }

    C4_DISABLED_FOR_CONST_C(void) reverse_range(size_t ifirst, size_t ilast)
    {
        C4_ASSERT(ifirst >= 0 && ifirst <= len);
        C4_ASSERT(ilast  >= 0 && ilast  <= len);
        if(ifirst == ilast) return;
        _do_reverse(str + ifirst, str + ilast - 1);
    }

public:

    C4_DISABLED_FOR_CONST_C(basic_substring) erase(size_t pos, size_t num)
    {
        C4_ASSERT(pos >= 0 && pos+num <= len);
        size_t num_to_move = len - pos - num;
        memmove(str + pos, str + pos + num, sizeof(C) * num_to_move);
        return basic_substring{str, len - num};
    }

    C4_DISABLED_FOR_CONST_C(basic_substring) erase_range(size_t first, size_t last)
    {
        C4_ASSERT(first <= last);
        return erase(first, last-first);
    }

    C4_DISABLED_FOR_CONST_C(basic_substring) erase(basic_csubstr sub)
    {
        C4_ASSERT(contains(sub));
        C4_ASSERT(sub.str >= str);
        return erase(sub.str - str, sub.len);
    }

public:

    C4_DISABLED_FOR_CONST_C(bool) replace_all(C value, C repl, size_t pos=0)
    {
        bool did_it = false;
        while((pos = find(value, pos)) != npos)
        {
            str[pos++] = repl;
            did_it = true;
        }
        return did_it;
    }

#undef C4_DISABLED_FOR_CONST_C
#undef C4_NC2C
}; // template class basic_substring


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< typename C, size_t N > inline bool operator== (basic_substring<const C> const that, const C (&s)[N]) { return that.compare(s) == 0; }
template< typename C, size_t N > inline bool operator!= (basic_substring<const C> const that, const C (&s)[N]) { return that.compare(s) != 0; }
template< typename C, size_t N > inline bool operator<  (basic_substring<const C> const that, const C (&s)[N]) { return that.compare(s) <  0; }
template< typename C, size_t N > inline bool operator>  (basic_substring<const C> const that, const C (&s)[N]) { return that.compare(s) >  0; }
template< typename C, size_t N > inline bool operator<= (basic_substring<const C> const that, const C (&s)[N]) { return that.compare(s) <= 0; }
template< typename C, size_t N > inline bool operator>= (basic_substring<const C> const that, const C (&s)[N]) { return that.compare(s) >= 0; }

template< typename C, size_t N > inline bool operator== (const C (&s)[N], basic_substring<const C> const that) { return that.compare(s) == 0; }
template< typename C, size_t N > inline bool operator!= (const C (&s)[N], basic_substring<const C> const that) { return that.compare(s) != 0; }
template< typename C, size_t N > inline bool operator<  (const C (&s)[N], basic_substring<const C> const that) { return that.compare(s) >  0; }
template< typename C, size_t N > inline bool operator>  (const C (&s)[N], basic_substring<const C> const that) { return that.compare(s) <  0; }
template< typename C, size_t N > inline bool operator<= (const C (&s)[N], basic_substring<const C> const that) { return that.compare(s) >= 0; }
template< typename C, size_t N > inline bool operator>= (const C (&s)[N], basic_substring<const C> const that) { return that.compare(s) <= 0; }

template< typename C > inline bool operator== (basic_substring<const C> const that, C const c) { return that.compare(c) == 0; }
template< typename C > inline bool operator!= (basic_substring<const C> const that, C const c) { return that.compare(c) != 0; }
template< typename C > inline bool operator<  (basic_substring<const C> const that, C const c) { return that.compare(c) <  0; }
template< typename C > inline bool operator>  (basic_substring<const C> const that, C const c) { return that.compare(c) >  0; }
template< typename C > inline bool operator<= (basic_substring<const C> const that, C const c) { return that.compare(c) <= 0; }
template< typename C > inline bool operator>= (basic_substring<const C> const that, C const c) { return that.compare(c) >= 0; }

template< typename C > inline bool operator== (C const c, basic_substring<const C> const that) { return that.compare(c) == 0; }
template< typename C > inline bool operator!= (C const c, basic_substring<const C> const that) { return that.compare(c) != 0; }
template< typename C > inline bool operator<  (C const c, basic_substring<const C> const that) { return that.compare(c) >  0; }
template< typename C > inline bool operator>  (C const c, basic_substring<const C> const that) { return that.compare(c) <  0; }
template< typename C > inline bool operator<= (C const c, basic_substring<const C> const that) { return that.compare(c) >= 0; }
template< typename C > inline bool operator>= (C const c, basic_substring<const C> const that) { return that.compare(c) <= 0; }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** Because of a C++ limitation, substr cannot provide simultaneous
 * overloads for constructing from a char[N] and a char*; the latter
 * will always be chosen by the compiler. So this specialization is
 * provided to simplify obtaining a substr from a char*. Being a
 * function has the advantage of making evident the strlen() cost.
 *
 * @see For a more detailed explanation on why the overloads cannot
 * coexist, see http://cplusplus.bordoon.com/specializeForCharacterArrays.html */
inline substr to_substr(char *s)
{
    return substr(s, s ? strlen(s) : 0);
}

/** Because of a C++ limitation, substr cannot provide simultaneous
 * overloads for constructing from a char[N] and a char*; the latter
 * will always be chosen by the compiler. So this specialization is
 * provided to simplify obtaining a substr from a char*. Being a
 * function has the advantage of making evident the strlen() cost.
 *
 * @see For a more detailed explanation on why the overloads cannot
 * coexist, see http://cplusplus.bordoon.com/specializeForCharacterArrays.html */
inline csubstr to_csubstr(char *s)
{
    return csubstr(s, s ? strlen(s) : 0);
}

/** Because of a C++ limitation, substr cannot provide simultaneous
 * overloads for constructing from a const char[N] and a const char*;
 * the latter will always be chosen by the compiler. So this
 * specialization is provided to simplify obtaining a substr from a
 * char*. Being a function has the advantage of making evident the
 * strlen() cost.
 *
 * @see For a more detailed explanation on why the overloads cannot
 * coexist, see http://cplusplus.bordoon.com/specializeForCharacterArrays.html */
inline csubstr to_csubstr(const char *s)
{
    return csubstr(s, s ? strlen(s) : 0);
}

C4_END_NAMESPACE(c4)

#endif /* _C4_SUBSTR_HPP_ */
