# FluczakSignalBus

This library provides a lightweight and flexible mechanism for event-driven programming using delegates in C++. It allows for binding free functions, member functions, and lambdas to events, managing these bindings dynamically, and emitting events to invoke all registered callbacks. This is especially useful for systems that rely on decoupled communication, such as game engines, GUI frameworks, or messaging systems.

## Key Features

- **Type-Safe Delegates**: Supports binding to functions, member functions, and lambdas with full type safety.
- **Signal Bus**: A centralized hub for registering, emitting, and unbinding events, making it easy to manage complex event systems.
- **Dynamic Binding**: Supports dynamic addition and removal of callbacks during runtime.
- **Extensibility**: Provides a clean and modular design for adding more features if needed.

---

# How to Use

### Example Use Case: Delegates

### Code Example

```cpp
#include <iostream>
#include "Delegate.hpp"  

// Define an event type
struct PrintEvent
{
    std::string message;
};

// A class that contains a member function for handling events
class Printer
{
public:
    void PrintMessage(const PrintEvent& event)
    {
        std::cout << "Printer (Member Function): " << event.message << std::endl;
    }

    // A member variable (callback) to store event handlers
    void PrintCallback(const PrintEvent& event)
    {
        std::cout << "Printer (Callback): " << event.message << std::endl;
    }
};

int main()
{
    Printer printer;  // Create an instance of Printer
    Delegate<void(const PrintEvent&)> delegate;  // Create a delegate for PrintEvent

    // Bind the member function PrintMessage to the delegate
    delegate.Bind<Printer, &Printer::PrintMessage>(&printer);

    // Emit the event and invoke the member function
    delegate(PrintEvent{"Hello from the member function!"});

    // Bind the member variable callback PrintCallback to the delegate
    delegate.Bind<Printer, &Printer::PrintCallback>(&printer);

    // Emit the event again, now invoking the callback (member variable)
    delegate(PrintEvent{"Hello from the member variable callback!"});

    return 0;
}
```

### Example Use Case: Messaging System (SignalBus)

Consider a scenario where we want to create a messaging system for sending notifications between components of an application.

### Code Example

```cpp
#include <iostream>
#include "SignalBus.hpp"  // Assuming the library is contained here

// Define an event type
struct MessageEvent
{
    std::string message;
};

// Define a receiver class
class Receiver
{
public:
    void OnMessageReceived(const MessageEvent& event)
    {
        std::cout << "Received message: " << event.message << std::endl;
    }
};

int main()
{
    SignalBus bus;         // Create a signal bus
    Receiver receiver;     // Create a receiver instance

    // Bind the receiver's member function to the MessageEvent
    bus.Bind<MessageEvent, Receiver, &Receiver::OnMessageReceived>(&receiver);

    // Emit a MessageEvent
    bus.Emit<MessageEvent>({"Hello, World!"});

    // Unbind the receiver's member function
    bus.Unbind<MessageEvent, Receiver, &Receiver::OnMessageReceived>(&receiver);

    // Try emitting the event again (no output expected)
    bus.Emit<MessageEvent>({"This won't be received"});
    
    return 0;
}