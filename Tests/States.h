#include <any>
#include <iostream>
#include <memory>
#include <vector>

#include "Pdfsm.h"
#include "3rdParty/Blinker.h"

// Signals.
static Blinker::Board<4> signalBoard;

static struct
{
	std::shared_ptr<Blinker::Signal> x = signalBoard.NewSignal("x");
	std::shared_ptr<Blinker::Signal> y = signalBoard.NewSignal("y");
	std::shared_ptr<Blinker::Signal> z = signalBoard.NewSignal("z");
} signals;

// Blackboard
struct Blackboard
{
	int updateCounterA = 0;
	int updateCounterB = 0;
	int updateCounterC = 0;

	int onEnterCounterA = 0;
	int onEnterCounterB = 0;
	int onEnterCounterC = 0;

	int onTerminateCounterA = 0;
	int onTerminateCounterB = 0;
	int onTerminateCounterC = 0;

	int onPauseCounterA = 0;
	int onPauseCounterB = 0;
	int onPauseCounterC = 0;

	int onResumeCounterA = 0;
	int onResumeCounterB = 0;
	int onResumeCounterC = 0;
};

// States
enum class S
{
	A,
	B,
	C,
	N
};

// Transition table.
static Pdfsm::TransitionTable<S> transitionTable = {
	{ S::A, { S::B, S::C } },
	{ S::B, { S::C } },
};

// BaseState with OnSignal.
template <auto S>
class BaseStateBehavior : public Pdfsm::StateBehavior<S>
{
protected:
	std::unique_ptr<Blinker::Connection<4>> signalConnection = nullptr;

public:
	void OnSetup() override
	{
		// Connects to interested signals on initialize.
		auto patterns = SubscribledSignalPatterns();
		if (patterns.size())
			signalConnection = signalBoard.Connect(patterns);
	}

	bool BeforeUpdate(const Pdfsm::Context& ctx) override
	{
		bool abort = false;
		if (signalConnection != nullptr)
		{
			// Poll signals, propagates to OnSignal.
			signalConnection->Poll([&](Blinker::SignalId signalId, std::any signalData) {
				abort = OnSignal(ctx, signalId, signalData);
			});
		}
		return abort;
	}

	// APIs To Override.

	virtual bool OnSignal(const Pdfsm::Context& ctx, Blinker::SignalId signalId,
		std::any signalData)
	{
		return false;
	}
	virtual std::vector<std::string_view> SubscribledSignalPatterns() const { return {}; }
};

class A : public BaseStateBehavior<S::A>
{
public:
	std::vector<std::string_view> SubscribledSignalPatterns() const override
	{
		return { "x", "y" };
	}
	// Jumps to B on signal x
	// Jumps to C on signal y
	bool OnSignal(const Pdfsm::Context& ctx, Blinker::SignalId signalId,
		std::any signalData) override
	{
		std::cout << "A: on signal: " << std::to_string(signalId) << std::endl;
		if (signalId == signals.x->Id())
		{
			GetHandler().Jump(ctx, S::B);
			return true;
		}
		else if (signalId == signals.y->Id())
		{
			GetHandler().Jump(ctx, S::C);
			return true;
		}
		return false;
	}

	void Update(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->updateCounterA++;
		std::cout << "A: on update" << std::endl;
	}

	void OnEnter(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onEnterCounterA++;
		std::cout << "A: on enter" << std::endl;
	}
	void OnTerminate(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onTerminateCounterA++;
		std::cout << "A: on terminate" << std::endl;
	}
	void OnPause(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onPauseCounterA++;
		std::cout << "A: on pause" << std::endl;
	}
	void OnResume(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onResumeCounterA++;
		std::cout << "A: on resume" << std::endl;
	}
};

class B : public BaseStateBehavior<S::B>
{
public:
	std::vector<std::string_view> SubscribledSignalPatterns() const override
	{
		return { "z" };
	}
	bool OnSignal(const Pdfsm::Context& ctx, Blinker::SignalId signalId,
		std::any signalData) override
	{
		std::cout << "B: on signal: " << std::to_string(signalId) << std::endl;
		if (signalId == signals.z->Id())
		{
			GetHandler().Jump(ctx, S::C);
			return true;
		}
		return false;
	}
	void Update(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->updateCounterB++;
		std::cout << "B: on update" << std::endl;
	}
	void OnEnter(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onEnterCounterB++;
		std::cout << "B: on enter" << std::endl;
	}
	void OnTerminate(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onTerminateCounterB++;
		std::cout << "B: on terminate" << std::endl;
	}
	void OnPause(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onPauseCounterB++;
		std::cout << "B: on pause" << std::endl;
	}
	void OnResume(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onResumeCounterB++;
		std::cout << "B: on resume" << std::endl;
	}
};

class C : public BaseStateBehavior<S::C>
{
public:
	void Update(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->updateCounterC++;
		std::cout << "C: on update" << std::endl;
	}
	void OnEnter(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onEnterCounterC++;
		std::cout << "C: on enter" << std::endl;
	}
	void OnTerminate(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onTerminateCounterC++;
		std::cout << "C: on terminate" << std::endl;
	}
	void OnPause(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onPauseCounterC++;
		std::cout << "C: on pause" << std::endl;
	}
	void OnResume(const Pdfsm::Context& ctx) override
	{
		auto bb = std::any_cast<std::shared_ptr<Blackboard>>(ctx.data);
		bb->onResumeCounterC++;
		std::cout << "C: on resume" << std::endl;
	}
};

// Behaviors table
static Pdfsm::BTable<S> behaviorTable = {
	std::make_unique<A>(),
	std::make_unique<B>(),
	std::make_unique<C>(),
};

// Entities
struct Entity
{
	Pdfsm::StateMachine<S> fsm;
};
