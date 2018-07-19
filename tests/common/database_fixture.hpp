#ifndef DATABASE_FIXTURE_HPP
#define DATABASE_FIXTURE_HPP

#include <appbase/application.hpp>
#include <golos/chain/database.hpp>

#include <golos/protocol/exceptions.hpp>

#include <fc/io/json.hpp>
#include <fc/smart_ref_impl.hpp>

#include <golos/plugins/debug_node/plugin.hpp>
#include <golos/plugins/account_history/plugin.hpp>

#include <graphene/utilities/key_conversion.hpp>

#include <iostream>

#define INITIAL_TEST_SUPPLY (10000000000ll)

extern uint32_t ( STEEMIT_TESTING_GENESIS_TIMESTAMP );

#define PUSH_TX \
   golos::chain::test::_push_transaction

#define PUSH_BLOCK \
   golos::chain::test::_push_block

// See below
#define REQUIRE_OP_VALIDATION_SUCCESS(op, field, value) { \
   const auto temp = op.field; \
   op.field = value; \
   op.validate(); \
   op.field = temp; \
}
#define REQUIRE_OP_EVALUATION_SUCCESS(op, field, value) { \
   const auto temp = op.field; \
   op.field = value; \
   trx.operations.back() = op; \
   op.field = temp; \
   db.push_transaction( trx, ~0 ); \
}

/*#define STEEMIT_REQUIRE_THROW( expr, exc_type ) {       \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "STEEMIT_REQUIRE_THROW begin "        \
         << req_throw_info << std::endl;                  \
   BOOST_REQUIRE_THROW( expr, exc_type );                 \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "STEEMIT_REQUIRE_THROW end "          \
         << req_throw_info << std::endl;                  \
}*/

#define STEEMIT_REQUIRE_THROW(expr, exc_type)          \
   BOOST_REQUIRE_THROW( expr, exc_type );

#define STEEMIT_CHECK_THROW(expr, exc_type) {             \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "STEEMIT_CHECK_THROW begin "           \
         << req_throw_info << std::endl;                  \
   BOOST_CHECK_THROW( expr, exc_type );                   \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "STEEMIT_CHECK_THROW end "             \
         << req_throw_info << std::endl;                  \
}

#define GOLOS_CHECK_THROW_PROPS_IMPL( S, E, C, TL )                                                     \
    try {                                                                                               \
        BOOST_TEST_PASSPOINT();                                                                         \
        S;                                                                                              \
        BOOST_##TL( "exception '" BOOST_STRINGIZE( E ) "' is expected, but no exception thrown" ); }    \
    catch( E const& ex ) {                                                                              \
        const fc::variant_object &props = ex.get_log().at(0).get_data();                                \
        try {                                                                                           \
            C;                                                                                          \
        } catch (const fc::exception& err) {                                                            \
            BOOST_##TL( "caught exception '" << err.name() << "' while check props:" <<                 \
                err.to_detail_string());                                                                \
        }                                                                                               \
    } catch ( ... ) {                                                                                   \
        try {                                                                                           \
            throw;                                                                                      \
        } catch (const fc::exception& ex) {                                                             \
            BOOST_##TL( "exception '" BOOST_STRINGIZE( E ) "' is expected, "                      \
                "but '" << ex.name() << "' is caught");                                                 \
        } catch (...) {                                                                                 \
            BOOST_##TL( "exception " BOOST_STRINGIZE( E ) " is expected, "                        \
                "but unknown is caught");                                                               \
        }                                                                                               \
    }                                                                                                   \

#define GOLOS_WARN_THROW_PROPS(S, E, C)          GOLOS_CHECK_THROW_PROPS_IMPL(S, E, C, WARN)
#define GOLOS_CHECK_THROW_PROPS(S, E, C)         GOLOS_CHECK_THROW_PROPS_IMPL(S, E, C, ERROR)
#define GOLOS_REQUIRE_THROW_PROPS(S, E, C)       GOLOS_CHECK_THROW_PROPS_IMPL(S, E, C, FAIL)

#define GOLOS_CHECK_NO_THROW_IMPL( S, TL )                                                              \
    try {                                                                                               \
        BOOST_TEST_PASSPOINT();                                                                         \
        S;                                                                                              \
    } catch( fc::exception const& ex ) {                                                                \
        BOOST_##TL("no exception expected, but '" << ex.name() << "' thrown: \n" <<                     \
            ex.to_detail_string());                                                                     \
    } catch ( ... ) {                                                                                   \
        BOOST_##TL("no exception expected, but unknown exception thrown");                              \
    }

#define GOLOS_WARN_NO_THROW(S)          GOLOS_CHECK_NO_THROW_IMPL(S, WARN)
#define GOLOS_CHECK_NO_THROW(S)         GOLOS_CHECK_NO_THROW_IMPL(S, ERROR)
#define GOLOS_REQUIRE_NO_THROW(S)       GOLOS_CHECK_NO_THROW_IMPL(S, FAIL)

template<typename exception>
struct ErrorValidator {};

using ErrorValidateFunc = std::function<void(const std::string&, const fc::variant& props)>;

#define CHECK_ERROR(exception, ...) [&](const std::string& name, const fc::variant& props) \
    {\
        ErrorValidator<exception> v; \
        v.validate(name, props, __VA_ARGS__); \
    }

template<>
struct ErrorValidator<golos::invalid_parameter> {
    void validate(const std::string& name, const fc::variant& props,
            const std::string& param) {
        BOOST_CHECK_EQUAL(name, "invalid_parameter");
        BOOST_CHECK_EQUAL(props["param"].get_string(), param);
    }
};

template<>
struct ErrorValidator<golos::insufficient_funds> {
    void validate(const std::string& name, const fc::variant& props,
            const std::string& account, const std::string& balance, const std::string& amount) {
        BOOST_CHECK_EQUAL(name, "insufficient_funds");
        BOOST_CHECK_EQUAL(props["account"].get_string(), account);
        BOOST_CHECK_EQUAL(props["balance"].get_string(), balance);
        BOOST_CHECK_EQUAL(props["required"].get_string(), amount);
    }
};

template<>
struct ErrorValidator<golos::protocol::tx_invalid_operation> {
    void validate(const std::string& name, const fc::variant& props,
            int index, ErrorValidateFunc validator = NULL) {
        BOOST_CHECK_EQUAL(name, "tx_invalid_operation");
        BOOST_CHECK_EQUAL(props["index"].as_int64(), index);
        if(validator) {
            validator(props["error"]["name"].get_string(), props["error"]["stack"][(size_t)0]["data"]);
        }
    }
};

template<>
struct ErrorValidator<golos::missing_object> {
    void validate(const std::string& name, const fc::variant& props,
            const std::string& type, const std::string& id) {
        BOOST_CHECK_EQUAL(name, "missing_object");
        BOOST_CHECK_EQUAL(props["type"].get_string(), type);
        BOOST_CHECK_EQUAL(props["id"].get_string(), id);
    }

    void validate(const std::string& name, const fc::variant& props,
            const std::string& type, const fc::variant_object& id) {
        BOOST_CHECK_EQUAL(name, "missing_object");
        BOOST_CHECK_EQUAL(props["type"].get_string(), type);
        BOOST_CHECK_EQUAL(props["id"].get_object(), id);
    }
};

template<>
struct ErrorValidator<golos::object_already_exist> {
    void validate(const std::string& name, const fc::variant& props,
            const std::string& type, const std::string& id) {
        BOOST_CHECK_EQUAL(name, "object_already_exist");
        BOOST_CHECK_EQUAL(props["type"].get_string(), type);
        BOOST_CHECK_EQUAL(props["id"].get_string(), id);
    }

    void validate(const std::string& name, const fc::variant& props,
            const std::string& type, const fc::variant_object& id) {
        BOOST_CHECK_EQUAL(name, "object_already_exist");
        BOOST_CHECK_EQUAL(props["type"].get_string(), type);
        BOOST_CHECK_EQUAL(props["id"].get_object(), id);
    }
};

template<>
struct ErrorValidator<golos::logic_exception> {
    void validate(const std::string& name, const fc::variant& props,
            golos::logic_exception::error_types err) {
        BOOST_CHECK_EQUAL(name, "logic_exception");
        BOOST_CHECK_EQUAL(props["errid"].get_string(),
            fc::reflector<golos::logic_exception::error_types>::to_string(err));
    }
};

template<>
struct ErrorValidator<golos::bandwidth_exception> {
    void validate(const std::string& name, const fc::variant& props,
            golos::bandwidth_exception::bandwidth_types type) {
        BOOST_CHECK_EQUAL(name, "bandwidth_exception");
        BOOST_CHECK_EQUAL(props["bandwidth"].get_string(),
            fc::reflector<golos::bandwidth_exception::bandwidth_types>::to_string(type));
        BOOST_CHECK_NO_THROW(props["now"].get_string());
        BOOST_CHECK_NO_THROW(props["next"].get_string());
    }
};


template<>
struct ErrorValidator<golos::protocol::tx_irrelevant_sig> {
    void validate(const std::string& name, const fc::variant& props, int) {
        BOOST_CHECK_EQUAL(name, "tx_irrelevant_sig");
    }
};


template<>
struct ErrorValidator<golos::protocol::tx_duplicate_sig> {
    void validate(const std::string& name, const fc::variant& props, int) {
        BOOST_CHECK_EQUAL(name, "tx_duplicate_sig");
    }
};

template<>
struct ErrorValidator<golos::protocol::tx_duplicate_transaction> {
    void validate(const std::string& name, const fc::variant& props, int) {
        BOOST_CHECK_EQUAL(name, "tx_duplicate_transaction");
    }
};

template<>
struct ErrorValidator<golos::protocol::tx_missing_posting_auth> {
    void validate(const std::string& name, const fc::variant& props, int) {
        BOOST_CHECK_EQUAL(name, "tx_missing_posting_auth");
    }
};

template<>
struct ErrorValidator<golos::protocol::tx_missing_active_auth> {
    void validate(const std::string& name, const fc::variant& props, int) {
        BOOST_CHECK_EQUAL(name, "tx_missing_active_auth");
    }
};

template<>
struct ErrorValidator<golos::protocol::tx_missing_owner_auth> {
    void validate(const std::string& name, const fc::variant& props, int) {
        BOOST_CHECK_EQUAL(name, "tx_missing_owner_auth");
    }
};

#define GOLOS_CHECK_ERROR_PROPS_IMPL( S, C, TL ) \
    GOLOS_CHECK_THROW_PROPS_IMPL(S, golos::golos_exception, C(ex.name(), ex.get_log().at(0).get_data()), TL)

#define GOLOS_WARN_ERROR_PROPS(S, C)          GOLOS_CHECK_ERROR_PROPS_IMPL(S, C, WARN)
#define GOLOS_CHECK_ERROR_PROPS(S, C)         GOLOS_CHECK_ERROR_PROPS_IMPL(S, C, ERROR)
#define GOLOS_REQUIRE_ERROR_PROPS(S, C)       GOLOS_CHECK_ERROR_PROPS_IMPL(S, C, FAIL)


///This simply resets v back to its default-constructed value. Requires v to have a working assingment operator and
/// default constructor.
#define RESET(v) v = decltype(v)()
///This allows me to build consecutive test cases. It's pretty ugly, but it works well enough for unit tests.
/// i.e. This allows a test on update_account to begin with the database at the end state of create_account.
#define INVOKE(test) ((struct test*)this)->test_method(); trx.clear()

#define PREP_ACTOR(name) \
   fc::ecc::private_key name ## _private_key = generate_private_key(BOOST_PP_STRINGIZE(name));   \
   fc::ecc::private_key name ## _post_key = generate_private_key(BOOST_PP_STRINGIZE(name) "_post"); \
   public_key_type name ## _public_key = name ## _private_key.get_public_key();

#define ACTOR(name) \
   PREP_ACTOR(name) \
   const auto& name = account_create(BOOST_PP_STRINGIZE(name), name ## _public_key, name ## _post_key.get_public_key()); \
   account_id_type name ## _id = name.id; (void)name ## _id;

#define GET_ACTOR(name) \
   fc::ecc::private_key name ## _private_key = generate_private_key(BOOST_PP_STRINGIZE(name)); \
   const account_object& name = get_account(BOOST_PP_STRINGIZE(name)); \
   account_id_type name ## _id = name.id; (void)name ## _id;

#define ACTORS_IMPL(r, data, elem) ACTOR(elem)
#define ACTORS(names) BOOST_PP_SEQ_FOR_EACH(ACTORS_IMPL, ~, names) \
   validate_database();


// Note: testnet and mainnet can have different asset names
#define ASSET(s) asset::from_string(s)

#define ASSET_GBG(x)   asset(int64_t((x)*1e3), SBD_SYMBOL)
#define ASSET_GOLOS(x) asset(int64_t((x)*1e3), STEEM_SYMBOL)
#define ASSET_GESTS(x) asset(int64_t((x)*1e6), VESTS_SYMBOL)


// get_vesting_share_price() is a dynamic value which depends on funds,
//   that is why comparision can be done only with some correction
#define GOLOS_VEST_REQUIRE_EQUAL(left, right) \
    BOOST_REQUIRE( \
        std::abs((left).amount.value - (right).amount.value) < 5 && \
        (left).symbol == (right).symbol \
    )


namespace fc {

std::ostream& operator<<(std::ostream& out, const fc::exception& e);
std::ostream& operator<<(std::ostream& out, const fc::time_point& v);
std::ostream& operator<<(std::ostream& out, const fc::uint128_t& v);
std::ostream& operator<<(std::ostream& out, const fc::fixed_string<fc::uint128_t>& v);
std::ostream& operator<<(std::ostream &out, const fc::variant_object &v);

template<typename T>
std::ostream& operator<<(std::ostream& out, const fc::safe<T> &v) {
    out << v.value;
    return out;
}

bool operator==(const fc::variant_object &left, const fc::variant_object &right);

} // namespace fc


namespace chainbase {

template<typename T>
std::ostream& operator<<(std::ostream& out, const object_id<T> &v) {
    out << v._id;
    return out;
}

} // namespace chainbase


namespace std {

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& out, const std::pair<T1,T2> &v) {
    out << "<" << v.first << ":" << v.second << ">";
    return out;
}

} // namespace std


namespace golos { namespace protocol {

std::ostream& operator<<(std::ostream& out, const asset& v);
std::ostream& operator<<(std::ostream& out, const price& v);

} } // namespace golos::protocol



#ifndef STEEMIT_INIT_PRIVATE_KEY
#  define STEEMIT_INIT_PRIVATE_KEY (fc::ecc::private_key::regenerate(fc::sha256::hash(BLOCKCHAIN_NAME)))
#endif

namespace golos { namespace chain {

        using namespace golos::protocol;

        struct database_fixture {
            // the reason we use an app is to exercise the indexes of built-in plugins
            chain::database* db;
            signed_transaction trx;
            fc::ecc::private_key init_account_priv_key = STEEMIT_INIT_PRIVATE_KEY;
            string debug_key = golos::utilities::key_to_wif(init_account_priv_key);
            public_key_type init_account_pub_key = init_account_priv_key.get_public_key();
            uint32_t default_skip = 0 | database::skip_undo_history_check | database::skip_authority_check;

            golos::plugins::chain::plugin* ch_plugin = nullptr;
            golos::plugins::debug_node::plugin* db_plugin = nullptr;
            golos::plugins::operation_history::plugin* oh_plugin = nullptr;
            golos::plugins::account_history::plugin* ah_plugin = nullptr;

            optional<fc::temp_directory> data_dir;
            bool skip_key_index_test = false;

            uint32_t anon_acct_count;

            database_fixture() {
            }

            virtual ~database_fixture();

            static fc::ecc::private_key generate_private_key(string seed);

            string generate_anon_acct_name();

            void initialize();
            void startup(bool generate_hardfork = true);

            void open_database();
            void close_database();

            void generate_block(
                uint32_t skip = 0,
                const fc::ecc::private_key& key = STEEMIT_INIT_PRIVATE_KEY,
                int miss_blocks = 0);

            /**
             * @brief Generates block_count blocks
             * @param block_count number of blocks to generate
             */
            void generate_blocks(uint32_t block_count);

            /**
             * @brief Generates blocks until the head block time matches or exceeds timestamp
             * @param timestamp target time to generate blocks until
             */
            void generate_blocks(fc::time_point_sec timestamp, bool miss_intermediate_blocks = true);

            const account_object &account_create(
                    const string &name,
                    const string &creator,
                    const private_key_type &creator_key,
                    const share_type &fee,
                    const public_key_type &key,
                    const public_key_type &post_key,
                    const string &json_metadata
            );

            const account_object &account_create(
                    const string &name,
                    const public_key_type &key,
                    const public_key_type &post_key
            );

            const account_object &account_create(
                    const string &name,
                    const public_key_type &key
            );


            const witness_object &witness_create(
                    const string &owner,
                    const private_key_type &owner_key,
                    const string &url,
                    const public_key_type &signing_key,
                    const share_type &fee
            );

            void fund(const string &account_name, const share_type &amount = 500000);

            void fund(const string &account_name, const asset &amount);

            void transfer(const string &from, const string &to, const share_type &steem);

            void convert(const string &account_name, const asset &amount);

            void vest(const string &from, const share_type &amount);

            void vest(const string &account, const asset &amount);

            void proxy(const string &account, const string &proxy);

            void set_price_feed(const price &new_price);

            const asset &get_balance(const string &account_name) const;

            void sign(signed_transaction &trx, const fc::ecc::private_key &key);

            vector<operation> get_last_operations(uint32_t ops);

            // we have producer_reward virtual op now, so get_last_operations
            // can be hard to use after generate_blocks.
            // this get_last_operations variant helps to get only required op type
            template<typename op_type>
            vector<op_type> get_last_operations(uint32_t num_ops) {
                vector<op_type> ops;
                const auto& acc_hist_idx = db->get_index<golos::plugins::account_history::account_history_index>().indices().get<by_id>();
                auto itr = acc_hist_idx.end();
                while (itr != acc_hist_idx.begin() && ops.size() < num_ops) {
                    itr--;
                    auto op = fc::raw::unpack<golos::chain::operation>(db->get(itr->op).serialized_op);
                    if (op.which() == operation::tag<op_type>::value)
                        ops.push_back(op.get<op_type>());
                }
                return ops;
            }

            void validate_database(void);


            // helpers to remove boilerplate code
        private:
            void tx_push_ops(signed_transaction& tx, operation op) {
                tx.operations.push_back(op);
            }
            template<typename... Ops>
            void tx_push_ops(signed_transaction& tx, operation op, Ops... ops) {
                tx.operations.push_back(op);
                tx_push_ops(tx, ops...);
            }

        public:
            template<typename... Ops>
            void sign_tx_with_ops(signed_transaction& tx, const fc::ecc::private_key& k, Ops... ops) {
                tx.clear();
                tx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
                tx_push_ops(tx, ops...);
                sign(tx, k);
            }

            template<typename... Ops>
            void push_tx_with_ops(signed_transaction& tx, const fc::ecc::private_key& k, Ops... ops) {
                sign_tx_with_ops(tx, k, ops...);
                db->push_transaction(tx, 0);
            }
        };

        struct clean_database_fixture : public database_fixture {
            clean_database_fixture();

            ~clean_database_fixture() override;

            void resize_shared_mem(uint64_t size);
        };

        struct live_database_fixture : public database_fixture {
            live_database_fixture();

            ~live_database_fixture() override;

            fc::path _chain_dir;
        };

        struct add_operations_database_fixture : public database_fixture {
            typedef golos::plugins::operation_history::plugin plugin_type;

            add_operations_database_fixture();
            ~add_operations_database_fixture() override;

            void add_operations();

            plugin_type* _plg;
            std::map<std::string, std::string> _added_ops;
        };

        struct add_accounts_database_fixture : public add_operations_database_fixture {
            typedef golos::plugins::account_history::plugin plugin_type;

            add_accounts_database_fixture();
            ~add_accounts_database_fixture() override;

            void add_accounts();

            plugin_type* _plg;
            fc::flat_set<golos::chain::account_name_type> _account_names;
            //fc::flat_map<std::string, std::string> _added_accounts;
        };

        namespace test {
            bool _push_block(database& db, const signed_block& b, uint32_t skip_flags = 0);

            void _push_transaction(database& db, const signed_transaction& tx, uint32_t skip_flags = 0);
        }

} } // golos:chain
#endif
