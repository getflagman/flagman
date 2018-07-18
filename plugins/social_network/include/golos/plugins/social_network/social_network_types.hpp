#pragma once

namespace golos { namespace plugins { namespace social_network {
    using namespace golos::chain;
    
    #ifndef SOCIAL_NETWORK_SPACE_ID
    #define SOCIAL_NETWORK_SPACE_ID 10
    #endif

        enum social_network_types {
            comment_content_object_type = (SOCIAL_NETWORK_SPACE_ID << 8),
            comment_reward_object_type = (SOCIAL_NETWORK_SPACE_ID << 8) + 1
        };


        class comment_content_object
                : public object<comment_content_object_type, comment_content_object> {
        public:
            comment_content_object() = delete;

            template<typename Constructor, typename Allocator>
            comment_content_object(Constructor&& c, allocator <Allocator> a)
                    :title(a), body(a), json_metadata(a) {
                c(*this);
            }

            id_type id;

            comment_id_type   comment;

            shared_string title;
            shared_string body;
            shared_string json_metadata;

            uint32_t block_number;
        };


        using comment_content_id_type = object_id<comment_content_object>;

        struct by_comment;
        struct by_block_number;

        typedef multi_index_container<
              comment_content_object,
              indexed_by<
                 ordered_unique<tag<by_id>, member<comment_content_object, comment_content_id_type, &comment_content_object::id>>,
                 ordered_unique<tag<by_comment>, member<comment_content_object, comment_id_type, &comment_content_object::comment>>,
                 ordered_non_unique<tag<by_block_number>, member<comment_content_object, uint32_t, &comment_content_object::block_number>>>,
            allocator<comment_content_object>
        > comment_content_index;

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
            share_type author_rewards = 0;
            asset author_gbg_payout_value{0, SBD_SYMBOL};
            asset author_golos_payout_value{0, STEEM_SYMBOL};
            asset author_gests_payout_value{0, VESTS_SYMBOL};
            asset beneficiary_payout_value{0, SBD_SYMBOL};
            asset beneficiary_gests_payout_value{0, VESTS_SYMBOL};
            asset curator_payout_value{0, SBD_SYMBOL};
            asset curator_gests_payout_value{0, VESTS_SYMBOL};
        };

        typedef object_id<comment_reward_object> comment_reward_id_type;

        typedef multi_index_container<
            comment_reward_object,
            indexed_by<
                ordered_unique<tag<by_id>, member<comment_reward_object, comment_reward_object::id_type, &comment_reward_object::id>>,
                ordered_unique<tag<by_comment>, member<comment_reward_object, comment_object::id_type, &comment_reward_object::comment>>>,
            allocator<comment_reward_object>
        > comment_reward_index;
} } }


CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::social_network::comment_content_object, 
    golos::plugins::social_network::comment_content_index
)

CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::social_network::comment_reward_object,
    golos::plugins::social_network::comment_reward_index)