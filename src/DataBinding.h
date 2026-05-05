#pragma once

#include "Utils.h"

#include <algorithm>
#include <concepts>
#include <functional>
#include <vector>

namespace gui {
    template <typename T>
    using OneWayBinding = utils::ValueChanged<const T&>;

    template <typename T>
    concept PropertyValue = std::copyable<T> && std::equality_comparable<T>;

    template <PropertyValue T>
    class Property {
    public:
        using Transformer = std::function<T(const T&)>;

        Property(const Property&) = delete;
        Property& operator=(const Property&) = delete;
        Property(Property&&) = delete;
        Property& operator=(Property&&) = delete;

        Property() = default;
        Property(T value)
            : m_value(std::move(value)) {}

        operator T() const { return m_value; }
        operator const T&() const { return m_value; }
        Property& operator=(const T& value) {
            Set(value);
            return *this;
        }
        const T& operator()() const { return m_value; }
        T& operator()() { return m_value; }

        // Math operators
        Property& operator++()
            requires std::integral<T> || std::floating_point<T>
        {
            Set(m_value + 1);
            return *this;
        }

        Property& operator--()
            requires std::integral<T> || std::floating_point<T>
        {
            Set(m_value - 1);
            return *this;
        }

        T operator++(int)
            requires std::integral<T> || std::floating_point<T>
        {
            T old = m_value;
            Set(m_value + 1);
            return old;
        }

        T operator--(int)
            requires std::integral<T> || std::floating_point<T>
        {
            T old = m_value;
            Set(m_value - 1);
            return old;
        }

        Property& operator+=(const T& v)
            requires std::integral<T> || std::floating_point<T>
        {
            Set(m_value + v);
            return *this;
        }

        Property& operator-=(const T& v)
            requires std::integral<T> || std::floating_point<T>
        {
            Set(m_value - v);
            return *this;
        }

        Property& operator*=(const T& v)
            requires std::integral<T> || std::floating_point<T>
        {
            Set(m_value * v);
            return *this;
        }

        Property& operator/=(const T& v)
            requires std::integral<T> || std::floating_point<T>
        {
            Set(m_value / v);
            return *this;
        }

        const T& Get() const { return m_value; }
        T& Get() { return m_value; }

        void Set(const T& value) {
            T newValue = m_transformer ? m_transformer(value) : value;

            if (m_updating || m_value == newValue)
                return;
            m_updating = true;

            m_value = newValue;
            NotifyAll();
            m_updating = false;
            if (m_onUpdate)
                m_onUpdate();
        }

        void Bind(T& target) {
            target = m_value;
            m_oneWayBindings.push_back([&target](const T& v) { target = v; });
        }

        void Bind(OneWayBinding<T> cb) { m_oneWayBindings.push_back(std::move(cb)); }

        void Bind(Property<T>& target) {
            auto alreadyBound = [](const std::vector<Property<T>*>& peers, Property<T>* p) {
                return std::find(peers.begin(), peers.end(), p) != peers.end();
            };
            if (alreadyBound(m_peers, &target))
                return;
            m_peers.push_back(&target);
            target.m_peers.push_back(this);
            target.m_value = m_value;
        }

        void NotifyAll() {
            for (auto* peer : m_peers)
                peer->Set(m_value);
            for (auto& cb : m_oneWayBindings)
                cb(m_value);
        }

        void SetOnUpdate(utils::VoidCallback onUpdate) { m_onUpdate = onUpdate; }
        void SetTransformer(Transformer transformer) { m_transformer = transformer; }

        // For when we store containers
        template <typename U>
        void PushBack(const U& elem)
            requires requires(T& c, U v) { c.push_back(v); }
        {
            T next = m_value;
            next.push_back(elem);
            Set(next);
        }

        template <typename U>
        void Insert(size_t index, const U& elem)
            requires requires(T& c, U v) { c.insert(c.begin(), v); }
        {
            T next = m_value;
            next.insert(next.begin() + index, elem);
            Set(next);
        }

        void EraseAt(size_t index)
            requires requires(T& c) {
                c.erase(c.begin());
                c.size();
            }
        {
            if (index >= m_value.size())
                return;
            T next = m_value;
            next.erase(next.begin() + static_cast<typename T::difference_type>(index));
            Set(next);
        }

        void Clear()
            requires requires(T& c) { c.clear(); }
        {
            Set(T{});
        }

        size_t Size() const
            requires requires(const T& c) { c.size(); }
        {
            return m_value.size();
        }

        auto& operator[](size_t index)
            requires requires(T& c) { c[index]; }
        {
            return m_value[index];
        }

        const auto& operator[](size_t index) const
            requires requires(const T& c) { c[index]; }
        {
            return m_value[index];
        }

    private:
        T m_value{};
        bool m_updating{false};
        std::vector<Property<T>*> m_peers;
        std::vector<OneWayBinding<T>> m_oneWayBindings;
        utils::VoidCallback m_onUpdate;
        Transformer m_transformer;
    };

    template <PropertyValue T>
    class Computed {
    public:
        using Compute = std::function<T()>;

        Computed(const Computed&) = delete;
        Computed& operator=(const Computed&) = delete;
        Computed(Computed&&) = delete;
        Computed& operator=(Computed&&) = delete;
        Computed() = default;

        template <PropertyValue... Deps>
        Computed(Compute fn, Property<Deps>&... deps)
            : m_compute(std::move(fn)),
              m_value(m_compute()) {
            (deps.Bind([this](const auto&) { Recompute(); }), ...);
        }

        operator const T&() const { return m_value; }
        const T& Get() const { return m_value; }
        const T& operator()() const { return m_value; }

        void Bind(T& target) {
            target = m_value;
            m_oneWayBindings.push_back([&target](const T& v) { target = v; });
        }
        void Bind(OneWayBinding<T> cb) { m_oneWayBindings.push_back(std::move(cb)); }
        void Bind(Property<T>& target) {
            target.Set(m_value);
            m_oneWayBindings.push_back([&target](const T& v) { target.Set(v); });
        }

        size_t Size() const
            requires requires(const T& c) { c.size(); }
        {
            return m_value.size();
        }

        auto& operator[](size_t index)
            requires requires(T& c) { c[index]; }
        {
            return m_value[index];
        }

        const auto& operator[](size_t index) const
            requires requires(const T& c) { c[index]; }
        {
            return m_value[index];
        }

    private:
        Compute m_compute;
        T m_value{};
        std::vector<OneWayBinding<T>> m_oneWayBindings;

        void Recompute() {
            if (!m_compute)
                return;
            T newValue = m_compute();
            if (newValue == m_value)
                return;
            m_value = std::move(newValue);
            for (auto& cb : m_oneWayBindings)
                cb(m_value);
        }
    };
} // namespace gui