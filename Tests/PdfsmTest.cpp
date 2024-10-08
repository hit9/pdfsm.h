#include "Pdfsm.h"

#include <catch2/catch_test_macros.hpp>

#include "States.h"

TEST_CASE("Pdfsm/1", "[Push and pop]")
{
	auto						  bb = std::make_shared<Blackboard>();
	auto						  ctx = Pdfsm::Context(bb);
	Pdfsm::StateMachineHandler<S> h(behaviorTable, transitionTable);
	Pdfsm::StateMachine<S>		  fsm;
	h.SetHandlingFsm(fsm, ctx);
	// initial jumps to A
	REQUIRE(bb->onEnterCounterA == 1);
	// push to B
	h.Push(ctx, S::B);
	REQUIRE(bb->onPauseCounterA == 1);
	REQUIRE(bb->onEnterCounterB == 1);
	// pops
	h.Pop(ctx);
	REQUIRE(bb->onResumeCounterA == 1);
	REQUIRE(bb->onTerminateCounterB == 1);
	h.ClearHandlingFsm();
}

TEST_CASE("Pdfsm/2", "[Jump Check]")
{
	auto						  bb = std::make_shared<Blackboard>();
	auto						  ctx = Pdfsm::Context(bb);
	Pdfsm::StateMachine<S>		  fsm;
	Pdfsm::StateMachineHandler<S> h(behaviorTable, transitionTable);
	h.SetHandlingFsm(fsm, ctx);
	// inits to A
	REQUIRE(bb->onEnterCounterA == 1);
	// jump to B
	h.Push(ctx, S::B);
	// can't jump back to A.
	REQUIRE_THROWS(h.Jump(ctx, S::A));
	h.ClearHandlingFsm();
}

TEST_CASE("Pdfsm/3", "[Update]")
{
	auto						  bb = std::make_shared<Blackboard>();
	auto						  ctx = Pdfsm::Context(bb);
	Pdfsm::StateMachine<S>		  fsm;
	Pdfsm::StateMachineHandler<S> h(behaviorTable, transitionTable);
	h.SetHandlingFsm(fsm, ctx);
	// inits to A
	REQUIRE(bb->onEnterCounterA == 1);
	// Update
	h.Update(ctx);
	REQUIRE(bb->updateCounterA == 1);
	// Pushes B
	h.Push(ctx, S::B);
	h.Update(ctx);
	REQUIRE(bb->updateCounterA == 1);
	REQUIRE(bb->updateCounterB == 1);
	// Pushes C
	h.Push(ctx, S::C);
	h.Update(ctx);
	REQUIRE(bb->updateCounterA == 1);
	REQUIRE(bb->updateCounterB == 1);
	REQUIRE(bb->updateCounterC == 1);
	// Pops
	h.Pop(ctx);
	h.Update(ctx);
	REQUIRE(bb->updateCounterA == 1);
	REQUIRE(bb->updateCounterB == 2);
	REQUIRE(bb->updateCounterC == 1);
	h.ClearHandlingFsm();
}

TEST_CASE("Pdfsm/4", "[Signal]")
{
	signalBoard.Clear();
	auto						  bb = std::make_shared<Blackboard>();
	auto						  ctx = Pdfsm::Context(bb);
	Pdfsm::StateMachine<S>		  fsm;
	Pdfsm::StateMachineHandler<S> h(behaviorTable, transitionTable);
	h.SetHandlingFsm(fsm, ctx);

	// inits to A
	REQUIRE(bb->onEnterCounterA == 1);
	// Emit signal x and instant Flip.
	signals.x->Emit(0);
	signalBoard.Flip();
	// Update
	h.Update(ctx);
	REQUIRE(bb->updateCounterA == 0); // A misses this update.
	// Should jump to B.
	REQUIRE(h.Top() == S::B);
	// Emit signal z.
	signals.z->Emit(0);
	signalBoard.Flip();
	h.Update(ctx);
	REQUIRE(bb->updateCounterB == 0); // B misses this update.
	// Should jump to C.
	REQUIRE(h.Top() == S::C);

	h.ClearHandlingFsm();
}

TEST_CASE("Pdfsm/5", "[Multiple fsm]")
{
	auto						  bb = std::make_shared<Blackboard>();
	auto						  ctx = Pdfsm::Context(bb);
	Entity						  e1, e2;
	Pdfsm::StateMachineHandler<S> h(behaviorTable, transitionTable);
	// Handle fsm1
	h.SetHandlingFsm(e1.fsm, ctx);
	REQUIRE(bb->onEnterCounterA == 1);
	REQUIRE(h.Top() == S::A);
	h.Jump(ctx, S::B);
	REQUIRE(bb->onEnterCounterB == 1);
	REQUIRE(h.Top() == S::B);
	h.Update(ctx);
	REQUIRE(bb->updateCounterB == 1);
	h.ClearHandlingFsm();
	// Handle fsm2
	h.SetHandlingFsm(e2.fsm, ctx);
	REQUIRE(bb->onEnterCounterA == 2);
	REQUIRE(h.Top() == S::A);
	h.Jump(ctx, S::C);
	REQUIRE(bb->onEnterCounterC == 1);
	REQUIRE(h.Top() == S::C);
	h.Update(ctx);
	REQUIRE(bb->updateCounterC == 1);
	h.ClearHandlingFsm();
	// Back handling fsm1
	h.SetHandlingFsm(e1.fsm, ctx);
	REQUIRE(h.Top() == S::B); // still in B
	h.Update(ctx);
	REQUIRE(bb->updateCounterB == 2);
	h.ClearHandlingFsm();
	// Back handling fsm2
	h.SetHandlingFsm(e2.fsm, ctx);
	REQUIRE(h.Top() == S::C); // still in C
	h.Update(ctx);
	REQUIRE(bb->updateCounterC == 2);
	h.ClearHandlingFsm();
}
