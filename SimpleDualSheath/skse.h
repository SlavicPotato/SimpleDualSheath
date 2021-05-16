#pragma once

#include <ext/ISKSE.h>

class ISKSE :
    public ISKSEBase<
    SKSEInterfaceFlags::kMessaging |
    SKSEInterfaceFlags::kTrampoline |
    SKSEInterfaceFlags::kTask,
    24ui64,
    182ui64>
{
public:

    [[nodiscard]] SKMP_FORCEINLINE static auto& GetSingleton() {
        return m_Instance;
    }

    [[nodiscard]] SKMP_FORCEINLINE static auto& GetBranchTrampoline() {
        return m_Instance.GetTrampoline(TrampolineID::kBranch);
    }

    [[nodiscard]] SKMP_FORCEINLINE static auto& GetLocalTrampoline() {
        return m_Instance.GetTrampoline(TrampolineID::kLocal);
    }

private:
    ISKSE() = default;

    virtual void OnLogOpen() override;
    virtual const char* GetLogPath() const override;
    virtual const char* GetPluginName() const override;
    virtual UInt32 GetPluginVersion() const override;
    virtual bool CheckRuntimeVersion(UInt32 a_version) const;

    static ISKSE m_Instance;
};