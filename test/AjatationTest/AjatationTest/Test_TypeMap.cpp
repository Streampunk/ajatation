#include "stdafx.h"
#include "CppUnitTest.h"
#include "TypeMap.h"
#include "gen2ajaTypeMaps.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AjatationTest
{		
    typedef enum _TestEnum1
    {
        Enum1_none,
        Enum1_Val1,
        Enum1_Val2,
        Enum1_Val3,
        Enum1_Val4,
        Enum1_Val5,
        Enum1_Val6,
        Enum1_Val7
    } TestEnum1;

    typedef enum _TestEnum2
    {
        Enum2_none,
        Enum2_Val1,
        Enum2_Val2,
        Enum2_Val3,
        Enum2_Val4,
        Enum2_Val5,
        Enum2_Val6,
    } TestEnum2;

    typedef TypeMap<TestEnum1, TestEnum2> TestTypeMap;
    TestTypeMap::Entry testEntries[] = 
    {
        { Enum1_Val1, Enum2_Val5 },
        { Enum1_Val2, Enum2_Val4 },
        { Enum1_Val3, Enum2_Val3 },
        { Enum1_Val4, Enum2_Val2 },
        { Enum1_Val5, Enum2_Val1 },
        { Enum1_Val6, Enum2_Val1 },
    };

    TEST_CLASS(Test_TypeMap)
	{
	public:
		
		TEST_METHOD(TestInitialization)
		{
            TestTypeMap testMap(testEntries, Enum1_none, Enum2_none);

            validateMappingA2B(testMap, Enum1_Val1, Enum2_Val5);
            validateMappingA2B(testMap, Enum1_Val2, Enum2_Val4);
            validateMappingA2B(testMap, Enum1_Val3, Enum2_Val3);
            validateMappingA2B(testMap, Enum1_Val4, Enum2_Val2);
            validateMappingA2B(testMap, Enum1_Val5, Enum2_Val1);
            validateMappingA2B(testMap, Enum1_Val6, Enum2_Val1);
            validateMappingA2B(testMap, Enum1_Val7, Enum2_none);
            validateMappingA2B(testMap, Enum1_none, Enum2_none);

            validateMappingB2A(testMap, Enum2_Val1, Enum1_Val6);
            validateMappingB2A(testMap, Enum2_Val2, Enum1_Val4);
            validateMappingB2A(testMap, Enum2_Val3, Enum1_Val3);
            validateMappingB2A(testMap, Enum2_Val4, Enum1_Val2);
            validateMappingB2A(testMap, Enum2_Val5, Enum1_Val1);
            validateMappingB2A(testMap, Enum2_Val6, Enum1_none);
            validateMappingB2A(testMap, Enum2_none, Enum1_none);
        }

    private:

        template<typename A, typename B> void validateMappingA2B(TypeMap<A, B>& map, A input, B expected)
        {
            B value = map.ToB(input);

            Assert::AreEqual((int)value, (int)expected);
        }

        template<typename A, typename B> void validateMappingB2A(TypeMap<A, B>& map, B input, A expected)
        {
            Assert::AreEqual((int)map.ToA(input), (int)expected);
        }
    };
}