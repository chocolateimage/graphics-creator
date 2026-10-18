#pragma once
#include "../variant.hpp"
#include "ae_base.hpp"
#include "interaction_callbacks.hpp"

class AEEffect;
struct UtilityCallbacks;
struct SuiteLoader;

struct InData {
    InData();
    ~InData();

    InteractionCallbacks interactionCallbacks;
    UtilityCallbacks *utilityCallbacks;
    void *effect{nullptr};
    int32_t quality{0};
    int32_t version{0};
    int32_t serialNumber{0};
    int32_t applicationId{0};
    int32_t numberParams{0};
    int32_t reserved{0};
    int32_t whatCpu{0};
    int32_t whatFpu{0};
    int32_t currentTime{0};
    int32_t timeStep{0};
    int32_t totalTime{0};
    int32_t localTimeStep{0};
    uint32_t timeScale{0};
    int32_t field{0};
    int32_t shutterAngle{0};
    int32_t width{0};
    int32_t height{0};
    AERect extentHint{};
    int32_t originX{0};
    int32_t originY{0};
    Scale downsampleX{};
    Scale downsampleY{};
    Scale pixelAspectRatio{};
    int32_t flags{0};
    AEMemoryHandle *globalData{nullptr};
    AEMemoryHandle *sequenceData{nullptr};
    AEMemoryHandle *frameData{nullptr};
    int32_t sampleStart{0};
    int32_t sampleDuration{0};
    int32_t totalSample{0};
    char other_[4];
    char other[32];
    SuiteLoader *suiteLoader;
    int32_t preEffectOriginX{0};
    int32_t preEffectOriginY{0};
    int64_t shutterPhase{0};
} __attribute__((packed));

struct OutData {
    enum OutFlags {
        HasDialogs = 1 << 5,
        ErrorMessage = 1 << 8,
        Obsolete = 1 << 21,
    };
    enum OutFlags2 {
        Smart = 1 << 10,
    };

    uint32_t version{0};
    char name[32];
    AEMemoryHandle *globalData{nullptr};
    uint32_t numParams{0};
    AEMemoryHandle *sequenceData{nullptr};
    uint32_t unused1{0};
    AEMemoryHandle *frameData{nullptr};
    int32_t width{0};
    int32_t height{0};
    Vector2DInt offset{};
    uint32_t flags{0};
    char returnMessage[256];
    int32_t sampleStart{0};
    int32_t sampleDuration{0};
    void *other[4];
    uint32_t flags2{0};
};