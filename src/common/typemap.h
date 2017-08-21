/* Copyright 2017 Streampunk Media Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <map>

// Simple mapping class to map items of one type to another. 
//
template <typename A, typename B>
class TypeMap
{
public:

    typedef A A_type;
    typedef B B_type;

	struct Entry
	{
		A a_;
		B b_;

        Entry(A a, B b) : a_(a), b_(b) {}
	};

    template<int N> TypeMap(Entry (&entries) [N], A defaultA, B defaultB)
        : defaultA_(defaultA), defaultB_(defaultB)
    {
        for (int i = 0; i < N; ++i)
        {
            a2b_[entries[i].a_] = entries[i].b_;
            b2a_[entries[i].b_] = entries[i].a_;
        }
    }

    const A& ToA(const B& b) const
    {
        auto it = b2a_.find(b);

        if (it != b2a_.end())
        {
            return it->second;
        }

        return defaultA_;
    }

    const B& ToB(const A& a) const
    {
        auto it = a2b_.find(a);

        if (it != a2b_.end())
        {
            return it->second;
        }

        return defaultB_;
    }

private:

    std::map<A, B> a2b_;
    std::map<B, A> b2a_;
    A defaultA_;
    B defaultB_;
};