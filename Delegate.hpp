#pragma once
#include <exception>

template <typename Signature>
class Delegate;

/// @brief Exception thrown when a delegate call is attempted but the delegate is not properly bound.
class BadDelegateCall : public std::exception { };

/// @brief Abstract base class for all delegate handle types for proper storing of the delegate types and emitting inside the signal bus
struct IDelegateHandle
{
	virtual ~IDelegateHandle() = default;
};

/// @brief Represents a handle for managing delegates of a specific type inside signal bus
/// @tparam T The type of the event that the delegate is going to emit
template <typename T>
struct DelegateHandle : IDelegateHandle
{
	explicit DelegateHandle(const Delegate<void(const T&)>& delegate)
		: m_delegate(delegate) {}

	/// @brief Invokes the stored delegate with the provided event.
   /// @param event The data to pass to the delegate.
	void Emit(T event)
	{
		m_delegate(event);
	}

	/// @brief Checks if this delegate matches a specific instance and member function. Used for unbinding delegates from the signal bus
   /// @tparam Class The class type of the instance.
   /// @tparam MemberFunction The type of the member function to match.
   /// @param instance The instance to check for a match.
   /// @return True if the delegate matches the specified instance and member function; otherwise, false.
	template <typename Class, void (Class::* MemberFunction)(const T&)>
	bool Matches(const Class* instance) const
	{
		return m_delegate.Matches<Class, MemberFunction>(instance);
	}
private:
	Delegate<void(const T&)> m_delegate;
};


/// @brief Represents a type-safe callable delegate that can bind to functions, member functions, or lambdas.
/// @tparam R The return type of the delegate.
/// @tparam Args The argument types for the delegate.
template <typename R, typename...Args>
class Delegate<R(Args...)>
{
public:
	Delegate() = default;
	Delegate(const Delegate& other) = default;
	auto operator=(const Delegate& other)->Delegate & = default;

	/// @brief Invokes the delegate with the provided arguments.
	/// @param args The arguments to pass to the delegate.
	/// @return The result of the invocation.
	/// @throws BadDelegateCall if the delegate is not bound.
	R operator()(Args...args) const
	{
		if (m_stub == nullptr) 
		{
			throw BadDelegateCall{};
		}
		return (*m_stub)(m_instance, args...);
	}

	/// @brief Checks if this delegate matches a specific instance and member function. Used for unbinding and == checks.
	/// @tparam Class The class type of the instance.
	/// @tparam MemberFunction The type of the member function to match.
	/// @param instance The instance to check for a match.
	/// @return True if the delegate matches the specified instance and member function; otherwise, false.
	template <typename Class, R(Class::* MemberFunction)(Args...)>
	bool Matches(const Class* instance) const
	{
		return m_instance == instance && m_stub == [](const void* p, Args... args) -> R {
			auto* cls = const_cast<Class*>(static_cast<const Class*>(p));
			return (cls->*MemberFunction)(args...);
			};
	}

	/// @brief Binds a non-member function to the delegate.
	/// @tparam Function The non-member function to bind.
	template <R(*Function)(Args...)>
	void Bind()
	{
		m_instance = nullptr;
		m_stub = static_cast<StubFunction>([](const void*, Args...args) -> R {
			return (*Function)(args...);
			});
	}

	/// @brief Binds a const member function to the delegate.
	/// @tparam Class The class type of the instance.
	/// @tparam MemberFunction The const member function to bind.
	/// @param classPointer The instance to bind to.
	template <typename Class, R(Class::* MemberFunction)(Args...) const>
	Delegate Bind(const Class* classPointer)
	{
		m_instance = classPointer; // store the class pointer
		m_stub = static_cast<StubFunction>([](const void* p, Args...args) -> R {
			const auto* castedClass = static_cast<const Class*>(p);

			return (castedClass->*MemberFunction)(args...);
			});
	}

	/// @brief Binds a non-const member function to the delegate.
	/// @tparam Class The class type of the instance.
	/// @tparam MemberFunction The non-const member function to bind.
	/// @param c The instance to bind to.
	template <typename Class, R(Class::* MemberFunction)(Args...)>
	auto Bind(Class* c) -> void {
		m_instance = c; // store the class pointer
		m_stub = static_cast<StubFunction>([](const void* p, Args...args) -> R {
			// Safe, because we know the pointer was bound to a non-const instance
			auto* cls = const_cast<Class*>(static_cast<const Class*>(p));

			return (cls->*MemberFunction)(args...);
			});
	}

private:
	/// @brief Helper function for binding non-member functions.
	/// @tparam Function The non-member function to bind.
	/// @param args The arguments to pass to the function.
	/// @return The result of the invocation.
	template <R(*Function)(Args...)>
	static R NonMemberStub(const void* /* unused */, Args...args)
	{
		return (*Function)(args...);
	}

	using StubFunction = R(*)(const void*, Args...);///< The type of the stub function used for invocation

	const void* m_instance = nullptr; ///< The instance bound to the delegate, if any.
	StubFunction m_stub = nullptr;///< The stub function used to invoke the delegate.
};