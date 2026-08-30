#pragma once

#include <CommandBuffer.hpp>
#include <Event.hpp>
#include <Engine.hpp>

#include <string>
#include <memory>


namespace LiquorChess {
    class Engine;

    class Interface {
    public:
        Interface() = delete;
        explicit Interface(std::basic_istream<char>* input, std::basic_ostream<char>* output);
        virtual ~Interface() = default;

        virtual void Init();
        void Run();

        template<typename T, typename ...Args>
        void PushToGUI(Args&& ...args)
        {
            outputEvents.Push(AllocateEvent<T>(std::forward<Args>(args)...));
        }

    protected:
        virtual std::string SerializeEvent(Event* event) = 0;
        virtual Event* DeserializeEvent(const std::string& event) = 0;

        template<typename T, typename ...Args>
        T* AllocateEvent(Args&& ...args)
        {
            T* event = new T(std::forward<Args>(args)...);
            event->type = Event::Of<T>();
            return event;
        }

        CompoundEvent* AllocateCompoundEvent(std::initializer_list<Event*> events)
        {
            CompoundEvent* event = new CompoundEvent(events);
            event->type = Event::Of<CompoundEvent>();
            return event;
        }

        void DeallocateEvent(Event* event)
        {
            delete event;
        }

    private:
        void ListenInputLoop();

        void HandleEvent(Event* event);

        void ReadyEngine();

    private:
        std::basic_istream<char>* input;
        std::basic_ostream<char>* output;

        CommandBuffer<Event*, 256> inputEvents;
        CommandBuffer<Event*, 256> outputEvents;

        std::atomic<bool> running{ false };

        std::unique_ptr<Engine> engine;
    };

}
