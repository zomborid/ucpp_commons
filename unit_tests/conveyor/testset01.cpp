
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "conveyor/conveyor.h"

SCENARIO("items can be put onto and received from a conveyor", "[conveyor]"){

	GIVEN("an empty conveyor"){

		Conveyor<char, 3> conveyor;

		REQUIRE( (conveyor.SLOTS == 7) );

		WHEN("a buffer is fetched"){
			auto fetched = conveyor.fetch();

			THEN("it is valid"){
				REQUIRE(fetched.isValid());
			}

			AND_WHEN("a buffer is dequeued"){
				auto dequeued = conveyor.dequeue();

				THEN("it is not valid"){
					REQUIRE_FALSE(dequeued.isValid());
				}
			}

			AND_WHEN("it is written and enqueued"){
				*fetched.p = '+';
				conveyor.enqueue(fetched);
				fetched.clear(); // only resets the handle

				AND_WHEN("a buffer is dequeued"){
					auto dequeued = conveyor.dequeue();

					THEN("it is valid"){
						REQUIRE(dequeued.isValid());

						AND_THEN("the contents are correct"){
							REQUIRE(*dequeued.p == '+');
						}
					}
				}
			}
		}

		WHEN("all available buffers are fetched"){
			Handle<char> handles[conveyor.SLOTS];
			for(int i = 0; i < conveyor.SLOTS; ++i){
				handles[i] = conveyor.fetch();
			}

			THEN("they are all valid"){
				bool all_valid = true;
				for(int i = 0; i < conveyor.SLOTS; ++i){
					all_valid &= handles[i].isValid();
				}
				REQUIRE(all_valid);
			}

			THEN("valid buffer can't be fetched"){
				REQUIRE_FALSE(conveyor.fetch().isValid());
			}

			AND_WHEN("the buffers are written and enqueued"){
				for(int i = 0; i < conveyor.SLOTS; ++i){
					*handles[i].p = i + 'A';
					conveyor.enqueue(handles[i]);
					handles[i].clear();
				}

				THEN("valid buffer can't be fetched"){
					REQUIRE_FALSE(conveyor.fetch().isValid());
				}

				AND_WHEN("the buffers are dequeued"){
					for(int i = 0; i < conveyor.SLOTS; ++i){
						handles[i] = conveyor.dequeue();
					}

					THEN("valid buffer can't be fetched"){
						REQUIRE_FALSE(conveyor.fetch().isValid());
					}

					THEN("they are all valid"){
						bool all_valid = true;
						for(int i = 0; i < conveyor.SLOTS; ++i){
							all_valid &= handles[i].isValid();
						}
						REQUIRE(all_valid);

						AND_THEN("their contents are correct"){
							bool all_correct = true;
							for(int i = 0; i < conveyor.SLOTS; ++i){
								all_correct &= *handles[i].p == (char)(i + 'A');
							}
							REQUIRE(all_correct);
						}
					}

					AND_WHEN("a buffer is recycled"){
						conveyor.recycle(handles[0]);
						handles[0].clear();

						THEN("a buffer can be fetched"){
							REQUIRE(conveyor.fetch().isValid());

							AND_THEN("no second buffer can be fetched"){
								REQUIRE_FALSE(conveyor.fetch().isValid());
							}
						}

						THEN("a buffer can be fetched and enqueued"){
							auto fetched = conveyor.fetch();
							REQUIRE(fetched.isValid());
							conveyor.enqueue(fetched);
							fetched.clear();

							AND_THEN("no second buffer can be fetched"){
								REQUIRE_FALSE(conveyor.fetch().isValid());
							}
						}
					}
				}
			}
		}

	}

}
