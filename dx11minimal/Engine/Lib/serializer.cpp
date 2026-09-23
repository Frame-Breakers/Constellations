#include "serializer.h"
#include "../../ecsHeaders.h"

using namespace std;
using json = nlohmann::json;

//////////////////////////////////////////////////////////////////////////////////

unordered_map<string, function<json(Component*)>> serializer::components;

void serializer::Initialize() {

    //BASIC COMPONENTS

    components[typeid(Transform).name()] = [](Component* comp) {
        auto* c = static_cast<Transform*>(comp);
        json j = *c;
        j["type"] = typeid(Transform).name();
        return j;
        };

    components[typeid(Transform2D).name()] = [](Component* comp) {
        auto* c = static_cast<Transform2D*>(comp);
        json j = *c;
        j["type"] = typeid(Transform2D).name();
        return j;
        };

    //PHYSIC

    components[typeid(PhysicBody).name()] = [](Component* comp) {
        auto* c = static_cast<PhysicBody*>(comp);
        json j = *c;
        j["type"] = typeid(PhysicBody).name();
        return j;
        };

    components[typeid(RotatingBody).name()] = [](Component* comp) {
        auto* c = static_cast<RotatingBody*>(comp);
        json j = *c;
        j["type"] = typeid(RotatingBody).name();
        return j;
        };

    components[typeid(SphereCollider).name()] = [](Component* comp) {
        auto* c = static_cast<SphereCollider*>(comp);
        json j = *c;
        j["type"] = typeid(SphereCollider).name();
        return j;
        };

    components[typeid(PlaneCollider).name()] = [](Component* comp) {
        auto* c = static_cast<PlaneCollider*>(comp);
        json j = *c;
        j["type"] = typeid(PlaneCollider).name();
        return j;
        };

    components[typeid(MeshCollider).name()] = [](Component* comp) {
        auto* c = static_cast<MeshCollider*>(comp);
        json j = *c;
        j["type"] = typeid(MeshCollider).name();
        return j;
        };

    components[typeid(GravityPoint).name()] = [](Component* comp) {
        auto* c = static_cast<GravityPoint*>(comp);
        json j = *c;
        j["type"] = typeid(GravityPoint).name();
        return j;
        };

    // DELAYED DESTROY

    components[typeid(DelayedDestroy).name()] = [](Component* comp) {
        auto* c = static_cast<DelayedDestroy*>(comp);
        json j = *c;
        j["type"] = typeid(DelayedDestroy).name();
        return j;
        };

    // COMBAT

    components[typeid(Health).name()] = [](Component* comp) {
        auto* c = static_cast<Health*>(comp);
        json j = *c;
        j["type"] = typeid(Health).name();
        return j;
        };

    components[typeid(SingleDamager).name()] = [](Component* comp) {
        auto* c = static_cast<SingleDamager*>(comp);
        json j = *c;
        j["type"] = typeid(SingleDamager).name();
        return j;
        };

    components[typeid(MultiDamager).name()] = [](Component* comp) {
        auto* c = static_cast<MultiDamager*>(comp);
        json j = *c;
        j["type"] = typeid(MultiDamager).name();
        return j;
        };

    components[typeid(RayDamager).name()] = [](Component* comp) {
        auto* c = static_cast<RayDamager*>(comp);
        json j = *c;
        j["type"] = typeid(RayDamager).name();
        return j;
        };

    components[typeid(DamageBlocker).name()] = [](Component* comp) {
        auto* c = static_cast<DamageBlocker*>(comp);
        json j = *c;
        j["type"] = typeid(DamageBlocker).name();
        return j;
        };

    // SOUND

    components[typeid(SoundPlayer).name()] = [](Component* comp) {
        auto* c = static_cast<SoundPlayer*>(comp);
        json j = *c;
        j["type"] = typeid(SoundPlayer).name();
        return j;
        };

    // RENDER

    components[typeid(Star).name()] = [](Component* comp) {
        auto* c = static_cast<Star*>(comp);
        json j = *c;
        j["type"] = typeid(Star).name();
        return j;
        };

    components[typeid(SpriteCluster).name()] = [](Component* comp) {
        auto* c = static_cast<SpriteCluster*>(comp);
        json j = *c;
        j["type"] = typeid(SpriteCluster).name();
        return j;
        };

    components[typeid(PointCloud).name()] = [](Component* comp) {
        auto* c = static_cast<PointCloud*>(comp);
        json j = *c;
        j["type"] = typeid(PointCloud).name();
        return j;
        };

    components[typeid(Constellation).name()] = [](Component* comp) {
        auto* c = static_cast<Constellation*>(comp);
        json j = *c;
        j["type"] = typeid(Constellation).name();
        return j;
        };

    components[typeid(Beam).name()] = [](Component* comp) {
        auto* c = static_cast<Beam*>(comp);
        json j = *c;
        j["type"] = typeid(Beam).name();
        return j;
        };

    components[typeid(StarClay).name()] = [](Component* comp) {
        auto* c = static_cast<StarClay*>(comp);
        json j = *c;
        j["type"] = typeid(StarClay).name();
        return j;
        };

    components[typeid(ParticleEmitter).name()] = [](Component* comp) {
        auto* c = static_cast<ParticleEmitter*>(comp);
        json j = *c;
        j["type"] = typeid(ParticleEmitter).name();
        return j;
        };

    components[typeid(Nebula).name()] = [](Component* comp) {
        auto* c = static_cast<Nebula*>(comp);
        json j = *c;
        j["type"] = typeid(Nebula).name();
        return j;
        };

    components[typeid(Mesh).name()] = [](Component* comp) {
        auto* c = static_cast<Mesh*>(comp);
        json j = *c;
        j["type"] = typeid(Mesh).name();
        return j;
        };
}

//////////////////////////////////////////////////////////////////////////////////

unordered_map<string, function<void(Entity*, const json&)>> deserializer::components;

void deserializer::Initialize() {

    //BASIC COMPONENTS

    components[typeid(Transform).name()] = [](Entity* entity, const json& data) {
        Transform* c = entity->AddComponent<Transform>();
        *c = data.get<Transform>();
        };

    components[typeid(Transform2D).name()] = [](Entity* entity, const json& data) {
        Transform2D* c = entity->AddComponent<Transform2D>();
        *c = data.get<Transform2D>();
        };

    //PHYSIC

    components[typeid(PhysicBody).name()] = [](Entity* entity, const json& data) {
        PhysicBody* c = entity->AddComponent<PhysicBody>();
        *c = data.get<PhysicBody>();
        };

    components[typeid(RotatingBody).name()] = [](Entity* entity, const json& data) {
        RotatingBody* c = entity->AddComponent<RotatingBody>();
        *c = data.get<RotatingBody>();
        };

    components[typeid(SphereCollider).name()] = [](Entity* entity, const json& data) {
        SphereCollider* c = entity->AddComponent<SphereCollider>();
        *c = data.get<SphereCollider>();
        };

    components[typeid(PlaneCollider).name()] = [](Entity* entity, const json& data) {
        PlaneCollider* c = entity->AddComponent<PlaneCollider>();
        *c = data.get<PlaneCollider>();
        };

    components[typeid(MeshCollider).name()] = [](Entity* entity, const json& data) {
        MeshCollider* c = entity->AddComponent<MeshCollider>();
        *c = data.get<MeshCollider>();
        };

    components[typeid(GravityPoint).name()] = [](Entity* entity, const json& data) {
        GravityPoint* c = entity->AddComponent<GravityPoint>();
        *c = data.get<GravityPoint>();
        };

    // DELAYED DESTROY

    components[typeid(DelayedDestroy).name()] = [](Entity* entity, const json& data) {
        DelayedDestroy* c = entity->AddComponent<DelayedDestroy>();
        *c = data.get<DelayedDestroy>();
        };

    // COMBAT

    components[typeid(Health).name()] = [](Entity* entity, const json& data) {
        Health* c = entity->AddComponent<Health>();
        *c = data.get<Health>();
        };

    components[typeid(SingleDamager).name()] = [](Entity* entity, const json& data) {
        SingleDamager* c = entity->AddComponent<SingleDamager>();
        *c = data.get<SingleDamager>();
        };

    components[typeid(MultiDamager).name()] = [](Entity* entity, const json& data) {
        MultiDamager* c = entity->AddComponent<MultiDamager>();
        *c = data.get<MultiDamager>();
        };

    components[typeid(RayDamager).name()] = [](Entity* entity, const json& data) {
        RayDamager* c = entity->AddComponent<RayDamager>();
        *c = data.get<RayDamager>();
        };

    components[typeid(DamageBlocker).name()] = [](Entity* entity, const json& data) {
        DamageBlocker* c = entity->AddComponent<DamageBlocker>();
        *c = data.get<DamageBlocker>();
        };
    
    // SOUND

    components[typeid(SoundPlayer).name()] = [](Entity* entity, const json& data) {
        SoundPlayer* c = entity->AddComponent<SoundPlayer>();
        *c = data.get<SoundPlayer>();
        };

    // RENDER

    components[typeid(Star).name()] = [](Entity* entity, const json& data) {
        Star* c = entity->AddComponent<Star>();
        *c = data.get<Star>();
        };

    components[typeid(SpriteCluster).name()] = [](Entity* entity, const json& data) {
        SpriteCluster* c = entity->AddComponent<SpriteCluster>();
        *c = data.get<SpriteCluster>();
        };

    components[typeid(PointCloud).name()] = [](Entity* entity, const json& data) {
        PointCloud* c = entity->AddComponent<PointCloud>();
        *c = data.get<PointCloud>();
        };

    components[typeid(Constellation).name()] = [](Entity* entity, const json& data) {
        Constellation* c = entity->AddComponent<Constellation>();
        *c = data.get<Constellation>();
        };

    components[typeid(Beam).name()] = [](Entity* entity, const json& data) {
        Beam* c = entity->AddComponent<Beam>();
        *c = data.get<Beam>();
        };

    components[typeid(StarClay).name()] = [](Entity* entity, const json& data) {
        StarClay* c = entity->AddComponent<StarClay>();
        *c = data.get<StarClay>();
        };

    components[typeid(ParticleEmitter).name()] = [](Entity* entity, const json& data) {
        ParticleEmitter* c = entity->AddComponent<ParticleEmitter>();
        *c = data.get<ParticleEmitter>();
        };

    components[typeid(Nebula).name()] = [](Entity* entity, const json& data) {
        Nebula* c = entity->AddComponent<Nebula>();
        *c = data.get<Nebula>();
        };

    components[typeid(Mesh).name()] = [](Entity* entity, const json& data) {
        Mesh* c = entity->AddComponent<Mesh>();
        *c = data.get<Mesh>();
        };

}