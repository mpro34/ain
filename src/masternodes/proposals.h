// Copyright (c) DeFi Blockchain Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEFI_MASTERNODES_PROPOSALS_H
#define DEFI_MASTERNODES_PROPOSALS_H

#include <flushablestorage.h>
#include <masternodes/res.h>
#include <script/script.h>
#include <amount.h>
#include <serialize.h>
#include <uint256.h>
#include <type_traits>

using CPropId = uint256;
constexpr const uint8_t VOC_CYCLES = 2;
constexpr const uint8_t MAX_CYCLES = 3;

enum class CPropType : uint8_t {
    CommunityFundProposal   = 0x01,
    BlockRewardReallocation = 0x02,
    VoteOfConfidence        = 0x03,
};

enum class CPropStatusType : uint8_t {
    Voting    = 0x01,
    Rejected  = 0x02,
    Completed = 0x03,
};

enum class CPropVoteType : uint8_t {
    VoteYes     = 0x01,
    VoteNo      = 0x02,
    VoteNeutral = 0x03,
};

std::string CPropTypeToString(const CPropType status);
std::string CPropVoteToString(const CPropVoteType status);
std::string CPropStatusToString(const CPropStatusType status);

struct CCreatePropMessage {
    CPropType type;
    CScript address;
    CAmount nAmount;
    uint8_t nCycles;
    std::string title;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        uint8_t typeByte;
        if (ser_action.ForRead()) {
            READWRITE(typeByte);
            type = static_cast<CPropType>(typeByte);
        } else {
            typeByte = static_cast<uint8_t>(type);
            READWRITE(typeByte);
        }
        READWRITE(address);
        READWRITE(nAmount);
        READWRITE(nCycles);
        READWRITE(title);
    }
};

struct CPropVoteMessage {
    CPropId propId;
    uint256 masternodeId;
    CPropVoteType vote;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(propId);
        READWRITE(masternodeId);
        uint8_t voteByte;
        if (ser_action.ForRead()) {
            READWRITE(voteByte);
            vote = static_cast<CPropVoteType>(voteByte);
        } else {
            voteByte = static_cast<uint8_t>(vote);
            READWRITE(voteByte);
        }
    }
};

struct CPropObject : public CCreatePropMessage {
    // memory only
    uint8_t cycle;
    uint32_t creationHeight;
    uint32_t finalHeight;
    // memory only
    CPropStatusType status;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITEAS(CCreatePropMessage, *this);
        READWRITE(creationHeight);
        READWRITE(finalHeight);
    }
};

struct CMnVotePerCycle {
    CPropId propId;
    uint8_t cycle;
    uint256 masternodeId;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(propId);
        READWRITE(cycle);
        READWRITE(masternodeId);
    }
};

/// View for managing proposals and their data
class CPropsView : public virtual CStorageView
{
public:
    ~CPropsView() override = default;

    Res CreateProp(const CPropId& propId, uint32_t height, const CCreatePropMessage& prop);
    std::optional<CPropObject> GetProp(const CPropId& propId);
    Res UpdatePropCycle(const CPropId& propId, uint8_t cycle);
    Res UpdatePropStatus(const CPropId& propId, uint32_t height, CPropStatusType status);
    Res AddPropVote(const CPropId& propId, const uint256& masternodeId, CPropVoteType vote);
    std::optional<CPropVoteType> GetPropVote(const CPropId& propId, uint8_t cycle, const uint256& masternodeId);

    void ForEachProp(std::function<bool(CPropId const &, CPropObject const &)> callback, uint8_t status = 0);
    void ForEachPropVote(std::function<bool(CPropId const &, uint8_t, uint256 const &, CPropVoteType)> callback, CMnVotePerCycle const & start = {});
    void ForEachCycleProp(std::function<bool(CPropId const &, CPropObject const &)> callback, uint32_t height);

    Res SetVotingPeriod(uint32_t votingPeriod);
    uint32_t GetVotingPeriod();

    struct ByType   { static constexpr uint8_t prefix() { return 0x2B; } };
    struct ByCycle  { static constexpr uint8_t prefix() { return 0x2C; } };
    struct ByMnVote { static constexpr uint8_t prefix() { return 0x2D; } };
    struct ByStatus { static constexpr uint8_t prefix() { return 0x2E; } };
    struct ByVoting { static constexpr uint8_t prefix() { return 0x2F; } };
};

#endif // DEFI_MASTERNODES_PROPOSALS_H
