#ifndef GOLOS_COMMENT_API_OBJ_H
#define GOLOS_COMMENT_API_OBJ_H

#include <golos/chain/comment_object.hpp>
#include <golos/chain/database.hpp>
#include <vector>

namespace golos { namespace api {

    using namespace golos::chain;

    struct comment_api_object {
        comment_object::id_type id;

        std::string title;
        std::string body;
        std::string json_metadata;

        account_name_type parent_author;
        std::string parent_permlink;
        account_name_type author;
        std::string permlink;

        std::string category;

        time_point_sec last_update;
        time_point_sec created;
        time_point_sec active;
        time_point_sec last_payout;

        uint8_t depth = 0;
        uint32_t children = 0;

        uint128_t children_rshares2 = 0;

        share_type net_rshares;
        share_type abs_rshares;
        share_type vote_rshares;

        share_type children_abs_rshares;
        time_point_sec cashout_time;
        time_point_sec max_cashout_time;
        uint64_t total_vote_weight = 0;

        uint16_t reward_weight = 0;

        protocol::asset total_payout_value;
        protocol::asset curator_payout_value;
        protocol::asset curator_gests_payout_value;

        share_type author_rewards;
        protocol::asset author_gbg_payout_value;
        protocol::asset author_golos_payout_value;
        protocol::asset author_gests_payout_value;

        int32_t net_votes = 0;

        comment_mode mode = not_set;

        comment_object::id_type root_comment;
        
        string root_title;

        protocol::asset max_accepted_payout;
        uint16_t percent_steem_dollars = 0;
        bool allow_replies = 0;
        bool allow_votes = 0;
        bool allow_curation_rewards = 0;

        vector< protocol::beneficiary_route_type > beneficiaries;
    };

} } // golos::api

FC_REFLECT(
    (golos::api::comment_api_object),
    (id)(author)(permlink)(parent_author)(parent_permlink)(category)(title)(body)(json_metadata)(last_update)
    (created)(active)(last_payout)(depth)(children)(children_rshares2)(net_rshares)(abs_rshares)
    (vote_rshares)(children_abs_rshares)(cashout_time)(max_cashout_time)(total_vote_weight)
    (reward_weight)(total_payout_value)(curator_payout_value)(curator_gests_payout_value)(author_rewards)(author_gbg_payout_value)(author_golos_payout_value)(author_gests_payout_value)(net_votes)
    (mode)(root_comment)(root_title)(max_accepted_payout)(percent_steem_dollars)(allow_replies)(allow_votes)
    (allow_curation_rewards)(beneficiaries))

#endif //GOLOS_COMMENT_API_OBJ_H
