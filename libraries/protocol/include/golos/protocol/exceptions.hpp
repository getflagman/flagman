#pragma once

#include <fc/exception/exception.hpp>
#include <golos/protocol/protocol.hpp>

#define GOLOS_ASSERT_MESSAGE(FORMAT, ...) \
    FC_LOG_MESSAGE(error, FORMAT, __VA_ARGS__)

#define GOLOS_CTOR_ASSERT(expr, exception_type, exception_ctor) \
    if (!(expr)) { \
        exception_type _E; \
        exception_ctor(_E); \
        throw _E; \
    }

#define GOLOS_ASSERT(expr, exception_type, FORMAT, ...) \
    FC_MULTILINE_MACRO_BEGIN \
        if (!(expr)) { \
            throw exception_type(GOLOS_ASSERT_MESSAGE(FORMAT, __VA_ARGS__)); \
        } \
    FC_MULTILINE_MACRO_END

#define GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(TYPE, BASE, CODE, WHAT) \
    public: \
        enum code_enum { \
            code_value = CODE, \
        }; \
        TYPE(fc::log_message&& m) \
            : BASE(fc::move(m), CODE, BOOST_PP_STRINGIZE(TYPE), WHAT) {} \
        TYPE() \
            : BASE(CODE, BOOST_PP_STRINGIZE(TYPE), WHAT) {} \
        virtual std::shared_ptr<fc::exception> dynamic_copy_exception() const { \
            return std::make_shared<TYPE>(*this); \
        } \
        virtual NO_RETURN void dynamic_rethrow_exception() const { \
            if (code() == CODE) { \
                throw *this; \
            } else { \
                fc::exception::dynamic_rethrow_exception(); \
            } \
        } \
    protected: \
        TYPE(const BASE& c) \
            : BASE(c) {} \
        explicit TYPE(int64_t code, const std::string& name_value, const std::string& what_value) \
            : BASE(code, name_value, what_value) {} \
        explicit TYPE(fc::log_message&& m, int64_t code, const std::string& name_value, const std::string& what_value) \
            : BASE(std::move(m), code, name_value, what_value) {}

#define GOLOS_DECLARE_DERIVED_EXCEPTION(TYPE, BASE, CODE, WHAT) \
    class TYPE: public BASE { \
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(TYPE, BASE, CODE, WHAT) \
    };

#define GOLOS_CHECK_LOGIC(expr, TYPE, MSG, ...) \
        GOLOS_ASSERT(expr, golos::logic_exception, MSG, ("errid", TYPE)__VA_ARGS__)

#define GOLOS_DECLARE_PARAM(PARAM, GETTER) auto (PARAM) = [&]() {\
    try {return (GETTER);} \
    catch (const fc::exception &e) { \
        FC_THROW_EXCEPTION(golos::invalid_parameter, \
            "Invalid parameter \"${param}\": ${errmsg}", \
            ("param", BOOST_PP_STRINGIZE(PARAM)) \
            ("errmsg", e.to_string()) \
            ("error", e) \
            ); \
    } \
}()

#define GOLOS_CHECK_PARAM(PARAM, VALIDATOR) \
    FC_MULTILINE_MACRO_BEGIN \
        try { \
            VALIDATOR; \
        } catch (const golos::invalid_value& e) { \
            FC_THROW_EXCEPTION(golos::invalid_parameter, "Invalid value \"${value}\" for parameter \"${param}\": ${errmsg}", \
                    ("param", FC_STRINGIZE(PARAM)) \
                    ("value", PARAM) \
                    ("errmsg", e.to_string()) \
                    ("error", static_cast<fc::exception>(e))); \
        } \
    FC_MULTILINE_MACRO_END

#define GOLOS_CHECK_OP_PARAM(OP, PARAM, VALIDATOR) \
    FC_MULTILINE_MACRO_BEGIN \
        try { \
            VALIDATOR; \
        } catch (const golos::invalid_value& e) { \
            FC_THROW_EXCEPTION(invalid_parameter, "Invalid value \"${value}\" for operation parameter \"${param}\": ${errmsg}", \
                    ("param", FC_STRINGIZE(PARAM)) \
                    ("value", OP.PARAM) \
                    ("errmsg", e.to_string()) \
                    ("error", static_cast<fc::exception>(e))); \
        } \
    FC_MULTILINE_MACRO_END

#define GOLOS_CHECK_VALUE(COND, MSG, ...) \
    GOLOS_ASSERT((COND), golos::invalid_value, MSG, __VA_ARGS__)

#define GOLOS_CHECK_LIMIT(limit, max_value) \
    GOLOS_ASSERT( limit <= max_value, golos::limit_too_large, "Exceeded limit value. Maximum allowed ${max}", \
            ("limit",limit)("max",max_value))

#define GOLOS_CHECK_ARGS_COUNT(args, required) \
    GOLOS_ASSERT( (args)->size() == (required), invalid_arguments_count, \
        "Expected ${required} argument(s), was ${count}", \
        ("count", (args)->size())("required", required) );

#define GOLOS_CHECK_OPT_ARGS_COUNT(args, min, max) \
    GOLOS_ASSERT( (args)->size() >= (min) && (args)->size() <= max, invalid_arguments_count, \
        "Expected ${min}..${max} argument(s), was ${count}", \
        ("count", (args)->size())("min", min)("max", max) );


#define GOLOS_THROW_MISSING_OBJECT(type, id, ...) \
    FC_THROW_EXCEPTION(golos::missing_object, "Missing ${type} with id \"${id}\"", \
            ("type",type)("id",id) __VA_ARGS__)

#define GOLOS_THROW_OBJECT_ALREADY_EXIST(type, id, ...) \
    FC_THROW_EXCEPTION(golos::object_already_exist, "Object ${type} with id \"${id}\" already exists", \
            ("type",type)("id",id) __VA_ARGS__)


#define GOLOS_THROW_INTERNAL_ERROR(MSG, ...) \
    FC_THROW_EXCEPTION(golos::internal_error, MSG, __VA_ARGS__)

namespace golos {
    GOLOS_DECLARE_DERIVED_EXCEPTION(
        golos_exception, fc::exception,
              0, "golos base exception")

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        operation_exception, golos_exception,
        1000000, "Opertaion exception");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        unsupported_operation, operation_exception,
        1010000, "Unsupported operation");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        parameter_exception, operation_exception,
        1020000, "Parameter exception");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        invalid_arguments_count, parameter_exception,
        1020100, "Invalid argument count");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        missing_object, parameter_exception,
        1020200, "Missing object");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        object_already_exist, parameter_exception,
        1020300, "Object already exist");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        invalid_parameter, parameter_exception,
        1020400, "Invalid parameter value");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        business_exception, golos_exception,
        2000000, "Business logic error");

    class bandwidth_exception : public business_exception {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            bandwidth_exception, business_exception,
            2010000, "bandwidth exceeded error");
    public:
        enum bandwidth_types {
            post_bandwidth,
            comment_bandwidth,
            vote_bandwidth,
        };
    };

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        insufficient_funds, business_exception,
        2020000, "Account does not have sufficient funds")

    class logic_exception : public business_exception {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            logic_exception, business_exception,
            2030000, "business logic error");
    public:
        enum error_types {
            reached_limit_for_pending_withdraw_requests    = 1,
            parent_of_comment_cannot_change,
            parent_perlink_of_comment_cannot_change,

            // Vote operation
            voter_declined_voting_rights,
            account_is_currently_challenged,
            votes_are_not_allowed,
            does_not_have_voting_power,
            voting_weight_is_too_small,
            cannot_vote_after_payout,
            cannot_vote_within_last_minute_before_payout,
            cannot_vote_with_zero_rshares,
            voter_has_used_maximum_vote_changes,
            already_voted_in_similar_way,

            // Comment operation
            cannot_update_comment_because_nothing_changed,
            reached_comment_max_depth,
            replies_are_not_allowed,
            discussion_is_frozen,
            comment_is_archived,
            comment_editable_during_first_24_hours,

            // withdraw_vesting
            insufficient_fee_for_powerdown_registered_account,
            operation_would_not_change_vesting_withdraw_rate,

            // limit order
            limit_order_must_be_for_golos_gbg_market,
            price_feed_must_be_for_golos_gbg_market,
            cancelling_not_filled_order,
        };
    };
    
    GOLOS_DECLARE_DERIVED_EXCEPTION(
        internal_error, golos_exception,
        4000000, "internal error");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        assert_exception, internal_error,
        4010000, "assert exception");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        invalid_value, internal_error,
        4020000, "invalid value exception");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        limit_too_large, invalid_value,
        4020100, "Exceeded limit value");

} // golos


namespace golos { namespace protocol {
    GOLOS_DECLARE_DERIVED_EXCEPTION(
        transaction_exception, golos_exception,
        3000000, "transaction exception")

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        tx_invalid_operation, transaction_exception,
        3010000, "invalid operation in transaction");

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        tx_missing_authority, transaction_exception,
        3020000, "missing authority");

    class tx_missing_active_auth: public tx_missing_authority {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            tx_missing_active_auth, tx_missing_authority,
            3020100, "missing required active authority");
    public:
        std::vector<account_name_type> missing_accounts;
        std::vector<public_key_type> used_signatures;
    };

    class tx_missing_owner_auth: public tx_missing_authority {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            tx_missing_owner_auth, tx_missing_authority,
            3020200, "missing required owner authority");
    public:
        std::vector<account_name_type> missing_accounts;
        std::vector<public_key_type> used_signatures;
    };

    class tx_missing_posting_auth: public tx_missing_authority {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            tx_missing_posting_auth, tx_missing_authority,
            3020300, "missing required posting authority");
    public:
        std::vector<account_name_type> missing_accounts;
        std::vector<public_key_type> used_signatures;
    };

    class tx_missing_other_auth: public tx_missing_authority {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            tx_missing_other_auth, tx_missing_authority,
            3020400, "missing required other authority");
    public:
        std::vector<authority> missing_auths;
    };

    class tx_irrelevant_sig: public transaction_exception {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            tx_irrelevant_sig, transaction_exception,
            3030000, "irrelevant signature included");
    public:
        std::vector<public_key_type> unused_signatures;
    };

    class tx_duplicate_sig: public transaction_exception {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            tx_duplicate_sig, transaction_exception,
            3060000, "duplicate signature included");
    };

    class tx_irrelevant_approval: public transaction_exception {
        GOLOS_DECLARE_DERIVED_EXCEPTION_BODY(
            tx_irrelevant_approval, transaction_exception,
            3070000, "irrelevant approval included");
    public:
        std::vector<account_name_type> unused_approvals;
    };

    GOLOS_DECLARE_DERIVED_EXCEPTION(
        tx_duplicate_transaction, transaction_exception,
        3080000, "duplicate transaction");


} } // golos::protocol

FC_REFLECT_ENUM(golos::logic_exception::error_types, 
        (reached_limit_for_pending_withdraw_requests)
        (parent_of_comment_cannot_change)
        (parent_perlink_of_comment_cannot_change)

        // Vote operation
        (voter_declined_voting_rights)
        (account_is_currently_challenged)
        (votes_are_not_allowed)
        (does_not_have_voting_power)
        (voting_weight_is_too_small)
        (cannot_vote_after_payout)
        (cannot_vote_within_last_minute_before_payout)
        (cannot_vote_with_zero_rshares)
        (voter_has_used_maximum_vote_changes)
        (already_voted_in_similar_way)

        // Comment operation
        (cannot_update_comment_because_nothing_changed)
        (reached_comment_max_depth)
        (replies_are_not_allowed)
        (discussion_is_frozen)
        (comment_is_archived)
        (comment_editable_during_first_24_hours)

        // withdraw_vesting
        (insufficient_fee_for_powerdown_registered_account)
        (operation_would_not_change_vesting_withdraw_rate)

        // limit order
        (limit_order_must_be_for_golos_gbg_market)
        (price_feed_must_be_for_golos_gbg_market)
        (cancelling_not_filled_order)
);

FC_REFLECT_ENUM(golos::bandwidth_exception::bandwidth_types,
        (post_bandwidth)
        (comment_bandwidth)
        (vote_bandwidth)
);
