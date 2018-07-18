#pragma once

#include <appbase/application.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <golos/api/discussion.hpp>
#include <golos/plugins/follow/plugin.hpp>
#include <golos/api/account_vote.hpp>
#include <golos/api/vote_state.hpp>

namespace golos { namespace plugins { namespace social_network {
    using plugins::json_rpc::msg_pack;
    using golos::api::discussion;
    using golos::api::account_vote;
    using golos::api::vote_state;
    using golos::api::comment_api_object;
    using namespace golos::chain;

    DEFINE_API_ARGS(get_content,                msg_pack, discussion)
    DEFINE_API_ARGS(get_content_replies,        msg_pack, std::vector<discussion>)
    DEFINE_API_ARGS(get_all_content_replies,    msg_pack, std::vector<discussion>)
    DEFINE_API_ARGS(get_account_votes,          msg_pack, std::vector<account_vote>)
    DEFINE_API_ARGS(get_active_votes,           msg_pack, std::vector<vote_state>)
    DEFINE_API_ARGS(get_replies_by_last_update, msg_pack, std::vector<discussion>)

    class social_network final: public appbase::plugin<social_network> {
    public:
        APPBASE_PLUGIN_REQUIRES (
            (chain::plugin)
            (json_rpc::plugin)
        )

        DECLARE_API(
            (get_content)
            (get_content_replies)
            (get_all_content_replies)
            (get_account_votes)
            (get_active_votes)
            (get_replies_by_last_update)
        )

        social_network();
        ~social_network();

        void set_program_options(
            boost::program_options::options_description&,
            boost::program_options::options_description& config_file_options
        ) override;

        static const std::string& name();

        void plugin_initialize(const boost::program_options::variables_map& options) override;

        void plugin_startup() override;
        void plugin_shutdown() override;

    private:
        struct impl;
        std::unique_ptr<impl> pimpl;
    };

    //
    // Plugins should #define their SPACE_ID's so plugins with
    // conflicting SPACE_ID assignments can be compiled into the
    // same binary (by simply re-assigning some of the conflicting #defined
    // SPACE_ID's in a build script).
    //
    // Assignment of SPACE_ID's cannot be done at run-time because
    // various template automagic depends on them being known at compile
    // time.
    //
#ifndef SOCIAL_NETWORK_SPACE_ID
#define SOCIAL_NETWORK_SPACE_ID 10
#endif

    // Plugins need to define object type IDs such that they do not conflict
    // globally. If each plugin uses the upper 8 bits as a space identifier,
    // with 0 being for chain, then the lower 8 bits are free for each plugin
    // to define as they see fit.
    enum social_network_object_types {
        comment_reward_object_type = (SOCIAL_NETWORK_SPACE_ID << 8)
    };

    class comment_reward_object: public object<comment_reward_object_type, comment_reward_object> {
    public:
                comment_reward_object() = delete;
        template<typename Constructor, typename Allocator>
        comment_reward_object(Constructor&& c, allocator<Allocator> a) {
            c(*this);
        }

        id_type id;

        //comment_reward_object(author_reward_operation &a)
        //: author_gbg_payout_value(a.sbd_payout), author_golos_payout_value(a.steem_payout), author_gests_payout_value(a.vesting_payout){
        //}

        comment_id_type comment;
        asset total_payout_value{0, SBD_SYMBOL};
        asset author_rewards{0, STEEM_SYMBOL};
        asset author_gbg_payout_value{0, SBD_SYMBOL};
        asset author_golos_payout_value{0, STEEM_SYMBOL};
        asset author_gests_payout_value{0, VESTS_SYMBOL};
        asset beneficiary_payout_value{0, SBD_SYMBOL};
        asset beneficiary_gests_payout_value{0, VESTS_SYMBOL};
        asset curator_payout_value{0, SBD_SYMBOL};
        asset curator_gests_payout_value{0, VESTS_SYMBOL};
    };

    typedef object_id<comment_reward_object> comment_reward_id_type;

    struct by_comment;

    typedef multi_index_container<
          comment_reward_object,
          indexed_by<
             ordered_unique<tag<by_id>, member<comment_reward_object, comment_reward_object::id_type, &comment_reward_object::id>>,
             ordered_unique<tag<by_comment>, member<comment_reward_object, comment_object::id_type, &comment_reward_object::comment>>>,
        allocator<comment_reward_object>
    > comment_reward_index;

} } } // golos::plugins::social_network


CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::social_network::comment_reward_object, golos::plugins::social_network::comment_reward_index)