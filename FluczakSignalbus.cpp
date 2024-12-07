#include <iostream>

#include "SignalBus.hpp"


class StringEvent
{
public:
	std::string s;
};

class A
{
public:
	void Say(const StringEvent& s)
	{
		std::cout << s.s;
	}
};

int main()
{
	SignalBus bus;
	A a;
	bus.Bind<StringEvent, A, &A::Say>(&a);

	StringEvent event;
	event.s = "Test1";
	bus.Emit(event);
	event.s = "Test2";
	bus.Emit(event);

	
	return 0;
}