#include"pch.h"
#include<CppUnitTest.h>
#include<queue.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace QueueTests {
	TEST_CLASS(QueueBasicOps) {
		TEST_METHOD(DequeueEmptyQueue) {
			Queue queue = queueCreateStack(10, sizeof(char), ListInt8, false);
			Assert::AreNotEqual((double)ListNone,(double)queue.listType);
			char currentChar = 'k';
			Assert::AreEqual((double)DEQUEUE_QUEUE_EMPTY, (double)queueDequeue(&queue, &currentChar,sizeof(char)));
			Assert::AreEqual('k', currentChar);
			queueDestroy(&queue, NULL);
		}

		TEST_METHOD(EnqueueFullQueue) {
			Queue queue = queueCreateStack(10, sizeof(char), ListInt8, false);
			Assert::AreNotEqual((double)ListNone, (double)queue.listType);
			char currentChar = 'k';
			for (size_t i = 0; i < 10; i++)
				Assert::AreEqual((double)ENQUEUE_SUCCSESS,(double)queueEnqueue(&queue, &currentChar, sizeof(char)));
			Assert::AreEqual((double)ENQUEUE_QUEUE_FULL, (double)queueEnqueue(&queue, &currentChar, sizeof(char)));
			Assert::AreEqual(true, queue.queueIsFull);
			Assert::AreEqual((double)DEQUEUE_SUCCESS, (double)queueDequeue(&queue, &currentChar, sizeof(char)));
			Assert::AreEqual(false, queue.queueIsFull);
			Assert::AreEqual('k', currentChar);
		}
	};
}