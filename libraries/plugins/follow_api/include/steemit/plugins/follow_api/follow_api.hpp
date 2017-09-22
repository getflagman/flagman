#pragma once

#include <steemit/plugins/database_api/comment_api_obj.hpp>
#include <steemit/plugins/json_rpc/utility.hpp>
#include <steemit/plugins/follow/follow_objects.hpp>

namespace steemit {
    namespace plugins {
        namespace follow_api {

            using namespace chain;

            struct feed_entry {
                account_name_type             author;
                string                        permlink;
                vector< account_name_type >   reblog_by;
                time_point_sec                reblog_on;
                uint32_t                      entry_id = 0;
            };

            struct comment_feed_entry {
                database_api::comment_api_obj comment;
                vector< account_name_type >      reblog_by;
                time_point_sec                   reblog_on;
                uint32_t                         entry_id = 0;
            };

            struct blog_entry {
                account_name_type author;
                account_name_type permlink;
                account_name_type blog;
                time_point_sec    reblog_on;
                uint32_t          entry_id = 0;
            };

            struct comment_blog_entry {
                database_api::comment_api_obj comment;
                string                           blog;
                time_point_sec                   reblog_on;
                uint32_t                         entry_id = 0;
            };

            struct account_reputation {
                account_name_type             account;
                steemit::protocol::share_type reputation;
            };

            struct follow_api_object {
                account_name_type             follower;
                account_name_type             following;
                vector< follow::follow_type > what;
            };

            struct reblog_count {
                account_name_type author;
                uint32_t          count;
            };

            struct get_followers_args {
                account_name_type    account;
                account_name_type    start;
                follow::follow_type  type;
                uint32_t             limit = 1000;
            };

            struct get_followers_return {
                vector< follow_api_object > followers;
            };

            typedef get_followers_args get_following_args;

            struct get_following_return {
                vector< follow_api_object > following;
            };

            struct get_follow_count_args {
                account_name_type account;
            };

            struct get_follow_count_return {
                account_name_type account;
                uint32_t          follower_count ;
                uint32_t          following_count ;
            };

            struct get_feed_entries_args {
                account_name_type account;
                uint32_t          start_entry_id = 0;
                uint32_t          limit = 500;
            };

            struct get_feed_entries_return {
                vector< feed_entry > feed;
            };

            typedef get_feed_entries_args get_feed_args;

            struct get_feed_return {
                vector< comment_feed_entry > feed;
            };

            typedef get_feed_entries_args get_blog_entries_args;

            struct get_blog_entries_return {
                vector< blog_entry > blog;
            };

            typedef get_feed_entries_args get_blog_args;

            struct get_blog_return {
                vector< comment_blog_entry > blog;
            };

            struct get_account_reputations_args {
                account_name_type account_lower_bound;
                uint32_t          limit = 1000;
            };

            struct get_account_reputations_return {
                vector< account_reputation > reputations;
            };

            struct get_reblogged_by_args {
                account_name_type author;
                string            permlink;
            };

            struct get_reblogged_by_return {
                vector< account_name_type > accounts;
            };

            struct get_blog_authors_args {
                account_name_type blog_account;
            };

            struct get_blog_authors_return {
                vector< std::pair<account_name_type, uint32_t> > blog_authors;
            };


            class follow_api final  {
            public:
                constexpr static const char *__name__ = "follow_api";
                follow_api();
                ~follow_api()= default;
                DECLARE_API (
                        (get_followers)
                        (get_following)
                        (get_follow_count)
                        (get_feed_entries)
                        (get_feed)
                        (get_blog_entries)
                        (get_blog)
                        (get_account_reputations)
                        ///Gets list of accounts that have reblogged a particular post
                        (get_reblogged_by)
                        /// Gets a list of authors that have had their content reblogged on a given blog account
                        (get_blog_authors)
                )

            private:
                struct follow_api_impl;
                std::shared_ptr<follow_api_impl> my;
            };

        }
    }
} // steemit::follow_api

FC_REFLECT( steemit::plugins::follow_api::feed_entry,
            (author)(permlink)(reblog_by)(reblog_on)(entry_id) );

FC_REFLECT( steemit::plugins::follow_api::comment_feed_entry,
            (comment)(reblog_by)(reblog_on)(entry_id) );

FC_REFLECT( steemit::plugins::follow_api::blog_entry,
            (author)(permlink)(blog)(reblog_on)(entry_id) );

FC_REFLECT( steemit::plugins::follow_api::comment_blog_entry,
            (comment)(blog)(reblog_on)(entry_id) );

FC_REFLECT( steemit::plugins::follow_api::account_reputation,
            (account)(reputation) );

FC_REFLECT( steemit::plugins::follow_api::follow_api_object,
            (follower)(following)(what) );

FC_REFLECT( steemit::plugins::follow_api::reblog_count,
            (author)(count) );

FC_REFLECT( steemit::plugins::follow_api::get_followers_args,
            (account)(start)(type)(limit) );

FC_REFLECT( steemit::plugins::follow_api::get_followers_return,
            (followers) );

FC_REFLECT( steemit::plugins::follow_api::get_following_return,
            (following) );

FC_REFLECT( steemit::plugins::follow_api::get_follow_count_args,
            (account) );

FC_REFLECT( steemit::plugins::follow_api::get_follow_count_return,
            (account)(follower_count)(following_count) );

FC_REFLECT( steemit::plugins::follow_api::get_feed_entries_args,
            (account)(start_entry_id)(limit) );

FC_REFLECT( steemit::plugins::follow_api::get_feed_entries_return,
            (feed) );

FC_REFLECT( steemit::plugins::follow_api::get_feed_return,
            (feed) );

FC_REFLECT( steemit::plugins::follow_api::get_blog_entries_return,
            (blog) );

FC_REFLECT( steemit::plugins::follow_api::get_blog_return,
            (blog) );

FC_REFLECT( steemit::plugins::follow_api::get_account_reputations_args,
            (account_lower_bound)(limit) );

FC_REFLECT( steemit::plugins::follow_api::get_account_reputations_return,
            (reputations) );

FC_REFLECT( steemit::plugins::follow_api::get_reblogged_by_args,
            (author)(permlink) );

FC_REFLECT( steemit::plugins::follow_api::get_reblogged_by_return,
            (accounts) );

FC_REFLECT( steemit::plugins::follow_api::get_blog_authors_args,
            (blog_account) );

FC_REFLECT( steemit::plugins::follow_api::get_blog_authors_return,
            (blog_authors) );
