#pragma once

#include <appbase/application.hpp>
#include <golos/plugins/chain/plugin.hpp>
#include <golos/api/discussion.hpp>
#include <golos/plugins/follow/plugin.hpp>
#include <golos/api/account_vote.hpp>
#include <golos/api/vote_state.hpp>
#include <golos/protocol/types.hpp>

namespace golos { namespace plugins { namespace social_network {
    using plugins::json_rpc::msg_pack;
    using golos::api::discussion;
    using golos::api::account_vote;
    using golos::api::vote_state;
    using golos::api::comment_api_object;
    using fc::time_point_sec;
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

    struct comment_last_update_object{
        comment_id_type comment;
        account_name_type parent_author;
        account_name_type author;
        time_point_sec last_update;
    };

} } } // golos::plugins::social_network