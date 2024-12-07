#pragma once
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "Delegate.hpp"


class SignalBus
{
public:
    /// @brief Emits an event to all bound delegates of the specified type.
   /// @tparam EventToEmit The type of the event to emit.
   /// @param data The event data to pass to the delegates.
    template <typename EventToEmit>
    void Emit(EventToEmit data)
    {
	    for (auto& element : m_map[typeid(EventToEmit)])
	    {
           auto casted= dynamic_cast<DelegateHandle<EventToEmit>*>( element.get());
           casted->Emit(data);
	    }
    }

    /// @brief Binds a member function of a specific class instance to an event.
    /// @tparam EventToBindInto The type of the event to bind to.
    /// @tparam ClassToBind The type of the class containing the member function.
    /// @tparam MemberFunction The member function to bind.
    /// @param instance A pointer to the instance of the class to bind.
    template <typename EventToBindInto, typename ClassToBind, void(ClassToBind::* MemberFunction)( const EventToBindInto&)>
    void Bind(ClassToBind* instance)
    {
        Delegate<void(const EventToBindInto&)> delegate;
        delegate.Bind<ClassToBind, MemberFunction>(instance);

        auto handle = std::make_unique<DelegateHandle<EventToBindInto>>(delegate);
        m_map[typeid(EventToBindInto)].push_back(std::move(handle));
    }

    /// @brief Unbinds a member function of a specific class instance from an event.
    /// @tparam EventToUnbind The type of the event to unbind from.
    /// @tparam ClassToUnbind The type of the class containing the member function.
    /// @tparam MemberFunction The member function to unbind.
    /// @param instance A pointer to the instance of the class to unbind.
	template <typename EventToUnbind, typename ClassToUnbind, void (ClassToUnbind::* MemberFunction)(const EventToUnbind&)>
	void Unbind(ClassToUnbind* instance)
    {
	    const auto it = m_map.find(typeid(EventToUnbind));
        if (it == m_map.end()) return; // No such event is bound

        // Remove handles that match the instance and member function
        it->second.erase(
            std::remove_if(
                it->second.begin(),
                it->second.end(),
                [instance](const std::unique_ptr<IDelegateHandle>& handle)
                {
                    // Attempt to cast to Handle<EventToUnbind>
                    auto* typedHandle = dynamic_cast<DelegateHandle<EventToUnbind>*>(handle.get());
                    if (!typedHandle) return false;

                    // Check if the delegate matches the instance
                    return typedHandle->Matches<ClassToUnbind, MemberFunction>(instance);
                }),
            it->second.end());

        if (!it->second.empty()) 
        {
            m_map.erase(it);
        }
    }

private:
    /// @brief A map that associates event types with a list of delegate handles.
	/// Each event type (key) maps to a vector of unique pointers to delegate handles.
    std::unordered_map<std::type_index, std::vector<std::unique_ptr<IDelegateHandle>>> m_map;
};


