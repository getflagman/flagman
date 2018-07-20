#pragma once

#include <golos/api/vote_state.hpp>
#include <golos/api/comment_api_object.hpp>


namespace golos { namespace api {

    struct discussion : public comment_api_object {
        discussion(const comment_api_object& o)
                : comment_api_object(o) {
            
        }

        discussion() {
        }

        string url; /// /category/@rootauthor/root_permlink#author/permlink
        asset pending_payout_value = asset(0, SBD_SYMBOL); ///< sbd
        asset pending_payout_gests_value = asset(0, VESTS_SYMBOL);
        asset pending_payout_golos_value = asset(0, STEEM_SYMBOL);
        asset total_pending_payout_value = asset(0, SBD_SYMBOL); ///< sbd including replies
        std::vector<vote_state> active_votes;
        uint32_t active_votes_count = 0;
        std::vector<string> replies; ///< author/slug mapping
        fc::optional<share_type> author_reputation;
        fc::optional<asset> promoted;
        double hot = 0;
        double trending = 0;
        uint32_t body_length = 0;
        std::vector<account_name_type> reblogged_by;
        optional <account_name_type> first_reblogged_by;
        optional <time_point_sec> first_reblogged_on;
    };

} } // golos::api

FC_REFLECT_DERIVED( (golos::api::discussion), ((golos::api::comment_api_object)),
        (url)(pending_payout_value)(pending_payout_gests_value)(pending_payout_golos_value)(total_pending_payout_value)(active_votes)(active_votes_count)(replies)
        (author_reputation)(promoted)(body_length)(reblogged_by)(first_reblogged_by)(first_reblogged_on))
