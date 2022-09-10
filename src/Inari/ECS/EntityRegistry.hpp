#pragma once

#include <any>
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace inari
{
    struct Entity
    {
        std::string uuid;
        std::string name;
    };
    using EntityPtr = std::shared_ptr<Entity>;

    class EntityRegistry
    {
        using ComponentHash = size_t;
        using AnyComponent = std::any;
        using ComponentMap = std::map<ComponentHash, AnyComponent>;

    public:
        EntityRegistry() = default;
        ~EntityRegistry() = default;

        EntityRegistry(EntityRegistry&&) = delete;
        EntityRegistry(const EntityRegistry&) = delete;

        EntityPtr createEntity(const std::string_view& name = "");

        EntityPtr getEntity(const std::string_view& name);
        const std::vector<EntityPtr>& getEntities() const { return m_entities; }

        bool destroyEntity(const EntityPtr& entity);
        bool destroyEntity(const std::string_view& name);

        template <class C, class... Args>
        bool emplaceComponent(const EntityPtr& entity, Args... args)
        {
            assert(entity != nullptr && "Entity is empty");

            auto& componentMap = m_collection[entity->uuid];
            ComponentHash hashCode = typeid(C).hash_code();
            if (componentMap.find(hashCode) == componentMap.end())
            {
                componentMap.emplace(hashCode, std::any(C { args... }));
            }
            return false;
        }

        template <class C>
        C* getComponent(const EntityPtr& entity)
        {
            assert(entity != nullptr && "Entity is empty");

            auto& entityIt = m_collection.find(entity->uuid);
            if (entityIt != m_collection.end())
            {
                auto& componentIt = entityIt->second.find(typeid(C).hash_code());
                if (componentIt != entityIt->second.end())
                {
                    return std::any_cast<C>(&componentIt->second);
                }
            }

            return nullptr;
        }

    private:
        std::vector<EntityPtr> m_entities;
        std::map<std::string, ComponentMap> m_collection;
    };
}