#pragma once

#include "UploadBuffer.hpp"

struct CBRecordBase {
    std::type_info* typeId = nullptr;

    CBRecordBase(std::type_info* typeId) : typeId(typeId) {}
};

template<typename T>
struct CBRecord : public CBRecordBase {
    using CBRecordBase::CBRecordBase;

    std::unique_ptr<UploadBuffer<T>> buffer = nullptr;
};

class CBPool {
  private:
    std::unordered_map<const std::type_info *, std::vector<shared_ptr<CBRecordBase>>> m_TypeToRecords;

    std::array<std::vector<std::shared_ptr<CBRecordBase>>, 3> m_ToRelease;

    int currFrameIndex = 0;

  public:
    template <typename T> std::shared_ptr<CBRecord<T>> get() {
        const auto *key = &typeid(T);
        auto& records = m_TypeToRecords[key];
        if (records.size() > 0) {
            return std::static_pointer_cast<CBRecord<T>>(records.pop_back()); 
        }

        return std::make_shared<CBRecord<T>>(key);
    }

    void release(std::shared_ptr<CBRecordBase> record) {
        m_ToRelease[currFrameIndex].push_back(record);
    }

    void beginFrame(int frameIndex) {
        for (auto record: m_ToRelease[frameIndex]) {
            m_TypeToRecords[record->typeId].push_back(record);
        }
        m_ToRelease.clear();

        currFrameIndex = frameIndex;
    }
}