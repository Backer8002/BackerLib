#include"pch.h"
#include <CppUnitTest.h>
#include<hashMap.h>
#include<iostream>
#include<stdio.h>
#include<random>
#include<unordered_map>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace hashTest {
	TEST_CLASS(hashTest) {
		TEST_METHOD(Random) {
			std::srand(time(NULL));
			for (size_t i = 0; i < 10000; i++)
			{
				uint64_t randomValue = (uint64_t)std::rand() * (uint64_t)std::rand();
				char buff[100];
				snprintf(buff, 100, "%016llX, %016llX\n", hashFunctionDefualt(1,8,reinterpret_cast<unsigned char*>(&randomValue)),randomValue);
				Logger::WriteMessage(buff);
			}
		}
		TEST_METHOD(Shifted) {
			for (size_t i = 0xff; i < 0x8000000000000000; i<<=1)
			{
				char buff[100];
				snprintf(buff, 100, "%016llX, %016llX\n", hashFunctionDefualt(1,8,reinterpret_cast<unsigned char*>(&i)), i);
				Logger::WriteMessage(buff);
			}
		}
		TEST_METHOD(Additive) {
			for (size_t i = 0; i < UINT16_MAX; i++)
			{
				char buff[100];
				snprintf(buff, 100, "%016llX, %016llX\n", hashFunctionDefualt(1, 8, reinterpret_cast<unsigned char*>(&i)), i);
				Logger::WriteMessage(buff);
			}
		}

		TEST_METHOD(Collision) {
			std::unordered_map<uint64_t, size_t> map;
			for (size_t i = 0; i < 0x7ffffffllu; i++)
			{
				auto hash = hashFunctionDefualt(1, 8, reinterpret_cast<unsigned char*>(&i));
				if (!map.try_emplace(hash, i).second) {
					char buff[100];
					snprintf(buff, 100, "Collision found: %016llX collided with %016llX\n", i, map[hash]);
					Logger::WriteMessage(buff);
				}
			}
		}
	};
}