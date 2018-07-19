#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/utilities/tempdir.hpp>
#include <golos/time/time.hpp>
#include <golos/chain/steem_objects.hpp>
#include <golos/plugins/account_history/history_object.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include "database_fixture.hpp"


#define STEEM_NAMESPACE_PREFIX std::string("golos::protocol::")

uint32_t STEEMIT_TESTING_GENESIS_TIMESTAMP = 1431700000;


namespace fc {

std::ostream& operator<<(std::ostream& out, const fc::exception& e) {
    out << e.to_detail_string();
    return out;
}

std::ostream& operator<<(std::ostream& out, const fc::time_point& v) {
    out << static_cast<std::string>(v);
    return out;
}

std::ostream& operator<<(std::ostream& out, const fc::uint128_t& v) {
    out << static_cast<std::string>(v);
    return out;
}

std::ostream& operator<<(std::ostream& out, const fc::fixed_string<fc::uint128_t>& v) {
    out << static_cast<std::string>(v);
    return out;
}

std::ostream& operator<<(std::ostream& out, const fc::variant_object &v) {
    out << fc::json::to_string(v);
    return out;
}

bool compare(const fc::variant &left, const fc::variant &right) {
    if (left.get_type() != right.get_type()) return false;
    switch (left.get_type()) {
        case variant::null_type:   return true;
        case variant::int64_type:  return left.as_int64() == right.as_int64();
        case variant::uint64_type: return left.as_uint64() == right.as_uint64();
        case variant::double_type: return left.as_double() == right.as_double();
        case variant::bool_type:   return left.as_bool() == right.as_bool();
        case variant::string_type: return left.get_string() == right.get_string();
        case variant::array_type:
            {
                const variants &l = left.get_array();
                const variants &r = right.get_array();
                if (l.size() != r.size()) return false;
                return std::equal(l.begin(), l.end(), r.begin(), compare);
            }
        case variant::object_type: return left.get_object() == right.get_object();
        case variant::blob_type:   return left.get_string() == right.get_string();
        default: return false;
    }
}

bool operator==(const fc::variant_object &left, const fc::variant_object &right) {
    if (left.size() != right.size()) return false;
    for (const auto &v: left) {
        const auto &ptr = right.find(v.key());
        if (ptr == right.end()) return false;
        if (false == compare(v.value(), ptr->value())) return false;
    }
    return true;
}

} // namespace fc


namespace golos { namespace protocol {

std::ostream& operator<<(std::ostream& out, const asset& v) {
    out << v.to_string();
    return out;
}

std::ostream& operator<<(std::ostream& out, const price& v) {
    out << v.base << '/' << v.quote << '=' << v.to_real();
    return out;
}

} } // namespace golos::protocol


namespace golos { namespace chain {

        using std::cout;
        using std::cerr;
        using namespace golos::plugins;
        using golos::plugins::json_rpc::msg_pack;


        database_fixture::~database_fixture() {
            if (db_plugin) {
                // clear all debug updates
                db_plugin->plugin_shutdown();
            }

            close_database();
        }

        clean_database_fixture::clean_database_fixture() {
            try {
                initialize();

                open_database();

                startup();
            } catch (const fc::exception &e) {
                edump((e.to_detail_string()));
                throw;
            }
        }

        clean_database_fixture::~clean_database_fixture() {
        }

        void clean_database_fixture::resize_shared_mem(uint64_t size) {
            db->wipe(data_dir->path(), data_dir->path(), true);
            int argc = boost::unit_test::framework::master_test_suite().argc;
            char **argv = boost::unit_test::framework::master_test_suite().argv;
            for (int i = 1; i < argc; i++) {
                const std::string arg = argv[i];
                if (arg == "--record-assert-trip") {
                    fc::enable_record_assert_trip = true;
                }
                if (arg == "--show-test-names") {
                    std::cout << "running test "
                              << boost::unit_test::framework::current_test_case().p_name
                              << std::endl;
                }
            }
            init_account_pub_key = init_account_priv_key.get_public_key();

            db->open(data_dir->path(), data_dir->path(), INITIAL_TEST_SUPPLY, size, chainbase::database::read_write);
            startup();
        }

        live_database_fixture::live_database_fixture() {
            try {
                ilog("Loading saved chain");
                _chain_dir = fc::current_path() / "test_blockchain";
                FC_ASSERT(fc::exists(_chain_dir), "Requires blockchain to test on in ./test_blockchain");

                initialize();

                db->open(_chain_dir, _chain_dir);
                golos::time::now();

                validate_database();
                generate_block();

                ilog("Done loading saved chain");
            }
            FC_LOG_AND_RETHROW()
        }

        live_database_fixture::~live_database_fixture() {
            try {
                db->pop_block();
                return;
            }
            FC_LOG_AND_RETHROW()
        }

        struct app_initialise {
            app_initialise() = default;
            ~app_initialise() = default;

            template<class plugin_type>
            plugin_type* get_plugin() {
                int argc = boost::unit_test::framework::master_test_suite().argc;
                char **argv = boost::unit_test::framework::master_test_suite().argv;
                for (int i = 1; i < argc; i++) {
                    const std::string arg = argv[i];
                    if (arg == "--record-assert-trip") {
                        fc::enable_record_assert_trip = true;
                    }
                    if (arg == "--show-test-names") {
                        std::cout << "running test "
                                  << boost::unit_test::framework::current_test_case().p_name
                                  << std::endl;
                    }
                }
                auto plg = &appbase::app().register_plugin<plugin_type>();
                appbase::app().initialize<plugin_type>(argc, argv);
                return plg;
            }
        };

        add_operations_database_fixture::add_operations_database_fixture() : _plg(nullptr) {
            try {
                ilog("add_operations_database_fixture: begin");

                _plg = app_initialise().get_plugin<plugin_type>();
                initialize();
                open_database();
            } catch (const fc::exception &e) {
                edump((e.to_detail_string()));
                throw;
            }
        }

        add_operations_database_fixture::~add_operations_database_fixture() try {
            ilog("add_operations_database_fixture: end");
        } FC_LOG_AND_RETHROW();

        void add_operations_database_fixture::add_operations() try {
            ACTORS((alice)(bob)(sam))
            fund("alice", 10000);
            vest("alice", 10000);
            fund("bob", 7500);
            vest("bob", 7500);
            fund("sam", 8000);
            vest("sam", 8000);

            db->set_clear_votes(0xFFFFFFFF);

            signed_transaction tx;

            comment_operation com;
            com.author = "bob";
            com.permlink = "test";
            com.parent_author = "";
            com.parent_permlink = "test";
            com.title = "foo";
            com.body = "bar";
            tx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
            tx.operations.push_back(com);
            tx.sign(bob_private_key, db->get_chain_id());
            db->push_transaction(tx, 0);
            generate_block();
            _added_ops.insert(std::make_pair(tx.id().str(), STEEM_NAMESPACE_PREFIX + "comment_operation"));
            ilog("Generate: " + tx.id().str() + " comment_operation");
            tx.operations.clear();
            tx.signatures.clear();

            vote_operation vote;
            vote.voter = "alice";
            vote.author = "bob";
            vote.permlink = "test";
            vote.weight = -1; ///< Nessary for the posiblity of delet_comment_operation.
            tx.operations.push_back(vote);
            vote.voter = "bob";
            tx.operations.push_back(vote);
            vote.voter = "sam";
            tx.operations.push_back(vote);
            tx.sign(alice_private_key, db->get_chain_id());
            tx.sign(bob_private_key, db->get_chain_id());
            tx.sign(sam_private_key, db->get_chain_id());
            db->push_transaction(tx, 0);
            generate_block();
            _added_ops.insert(std::make_pair(tx.id().str(), STEEM_NAMESPACE_PREFIX + "vote_operation"));
            ilog("Generate: " + tx.id().str() + " vote_operation");
            tx.operations.clear();
            tx.signatures.clear();

            delete_comment_operation dco;
            dco.author = "bob";
            dco.permlink = "test";
            tx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
            tx.operations.push_back(dco);
            tx.sign(bob_private_key, db->get_chain_id());
            db->push_transaction(tx, 0);
            generate_block();
            _added_ops.insert(std::make_pair(tx.id().str(), STEEM_NAMESPACE_PREFIX + "delete_comment_operation"));
            ilog("Generate: " + tx.id().str() + " delete_comment_operation");
            tx.operations.clear();
            tx.signatures.clear();

            account_create_operation aco;
            aco.new_account_name = "dave";
            aco.creator = STEEMIT_INIT_MINER_NAME;
            aco.owner = authority(1, init_account_pub_key, 1);
            aco.active = aco.owner;
            tx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
            tx.operations.push_back(aco);
            tx.sign(init_account_priv_key, db->get_chain_id());
            db->push_transaction(tx, 0);
            generate_block();
            _added_ops.insert(std::make_pair(tx.id().str(), STEEM_NAMESPACE_PREFIX + "account_create_operation"));
            ilog("Generate: " + tx.id().str() + " account_create_operation");
            tx.operations.clear();
            tx.signatures.clear();

            validate_database();
        } FC_LOG_AND_RETHROW();

        add_accounts_database_fixture::add_accounts_database_fixture() : _plg(nullptr) {
            try {
                ilog("add_accounts_database_fixture: begin");

                _account_names.insert("alice");
                _account_names.insert("bob");
                _account_names.insert("sam");
                _account_names.insert("dave");

                _plg = app_initialise().get_plugin<plugin_type>();
                initialize();
                open_database();
            } catch (const fc::exception &e) {
                edump((e.to_detail_string()));
                throw;
            }
        }

        add_accounts_database_fixture::~add_accounts_database_fixture() {
            ilog("add_accounts_database_fixture: end");
        }

        void add_accounts_database_fixture::add_accounts() try {
            add_operations_database_fixture::add_operations();
        } FC_LOG_AND_RETHROW();

        fc::ecc::private_key database_fixture::generate_private_key(string seed) {
            return fc::ecc::private_key::regenerate(fc::sha256::hash(seed));
        }

        string database_fixture::generate_anon_acct_name() {
            // names of the form "anon-acct-x123" ; the "x" is necessary
            //    to workaround issue #46
            return "anon-acct-x" + std::to_string(anon_acct_count++);
        }

        void database_fixture::initialize() {
            int argc = boost::unit_test::framework::master_test_suite().argc;
            char** argv = boost::unit_test::framework::master_test_suite().argv;
            for (int i = 1; i < argc; i++) {
                const std::string arg = argv[i];
                if (arg == "--record-assert-trip") {
                    fc::enable_record_assert_trip = true;
                }
                if (arg == "--show-test-names") {
                    std::cout << "running test "
                              << boost::unit_test::framework::current_test_case().p_name
                              << std::endl;
                }
            }

            ch_plugin = &appbase::app().register_plugin<golos::plugins::chain::plugin>();
            oh_plugin = &appbase::app().register_plugin<operation_history::plugin>();
            ah_plugin = &appbase::app().register_plugin<account_history::plugin>();
            db_plugin = &appbase::app().register_plugin<debug_node::plugin>();

            appbase::app().initialize<
                golos::plugins::chain::plugin,
                account_history::plugin,
                debug_node::plugin
            >(argc, argv);

            db_plugin->set_logging(false);

            db = &ch_plugin->db();
            BOOST_REQUIRE(db);
        }

        void database_fixture::startup(bool generate_hardfork) {
            generate_block();
            if (generate_hardfork) {
                db->set_hardfork(STEEMIT_NUM_HARDFORKS);
            }
            generate_block();

            vest(STEEMIT_INIT_MINER_NAME, 10000);

            // Fill up the rest of the required miners
            for (int i = STEEMIT_NUM_INIT_MINERS; i < STEEMIT_MAX_WITNESSES; i++) {
                account_create(STEEMIT_INIT_MINER_NAME + fc::to_string(i), init_account_pub_key);
                fund(STEEMIT_INIT_MINER_NAME + fc::to_string(i), STEEMIT_MIN_PRODUCER_REWARD.amount.value);
                witness_create(
                    STEEMIT_INIT_MINER_NAME + fc::to_string(i),
                    init_account_priv_key, "foo.bar", init_account_pub_key,
                    STEEMIT_MIN_PRODUCER_REWARD.amount);
            }

            oh_plugin->plugin_startup();
            ah_plugin->plugin_startup();
            db_plugin->plugin_startup();

            validate_database();
        }

        void database_fixture::open_database() {
            if (!data_dir) {
                data_dir = fc::temp_directory(golos::utilities::temp_directory_path());
                db->_log_hardforks = false;
                db->_is_testing = true;
                db->open(data_dir->path(), data_dir->path(), INITIAL_TEST_SUPPLY,
                    1024 * 1024 * 10, chainbase::database::read_write); // 10 MB file for testing
            }
        }

        void database_fixture::close_database() {
            if (data_dir) {
                db->wipe(data_dir->path(), data_dir->path(), true);
            }
        }

        void database_fixture::generate_block(uint32_t skip, const fc::ecc::private_key &key, int miss_blocks) {
            skip |= default_skip;
            msg_pack msg;
            msg.args = std::vector<fc::variant>({fc::variant(golos::utilities::key_to_wif(key)), fc::variant(1), fc::variant(skip), fc::variant(miss_blocks), fc::variant(true)});
            db_plugin->debug_generate_blocks(msg);
        }

        void database_fixture::generate_blocks(uint32_t block_count) {
            msg_pack msg;
            msg.args = std::vector<fc::variant>({fc::variant(debug_key), fc::variant(block_count), fc::variant(default_skip), fc::variant(0), fc::variant(true)});
            auto produced = db_plugin->debug_generate_blocks(msg);
            BOOST_REQUIRE(produced == block_count);
        }

        void database_fixture::generate_blocks(fc::time_point_sec timestamp, bool miss_intermediate_blocks) {
            msg_pack msg;
            msg.args = std::vector<fc::variant>({fc::variant(debug_key), fc::variant(timestamp), fc::variant(miss_intermediate_blocks), fc::variant(default_skip)});
            db_plugin->debug_generate_blocks_until(msg);
            BOOST_REQUIRE((db->head_block_time() - timestamp).to_seconds() < STEEMIT_BLOCK_INTERVAL);
        }

        const account_object &database_fixture::account_create(
                const string &name,
                const string &creator,
                const private_key_type &creator_key,
                const share_type &fee,
                const public_key_type &key,
                const public_key_type &post_key,
                const string &json_metadata
        ) {
            try {
                account_create_operation op;
                op.new_account_name = name;
                op.creator = creator;
                op.fee = fee;
                op.owner = authority(1, key, 1);
                op.active = authority(1, key, 1);
                op.posting = authority(1, post_key, 1);
                op.memo_key = key;
                op.json_metadata = json_metadata;

                trx.operations.push_back(op);
                trx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
                trx.sign(creator_key, db->get_chain_id());
                trx.validate();
                db->push_transaction(trx, 0);
                trx.operations.clear();
                trx.signatures.clear();

                const account_object &acct = db->get_account(name);

                return acct;
            }
            FC_CAPTURE_AND_RETHROW((name)(creator))
        }

        const account_object &database_fixture::account_create(
                const string &name,
                const public_key_type &key,
                const public_key_type &post_key
        ) {
            try {
                return account_create(
                        name,
                        STEEMIT_INIT_MINER_NAME,
                        init_account_priv_key,
                        30*1e3,
                        key,
                        post_key,
                        "");
            }
            FC_CAPTURE_AND_RETHROW((name));
        }

        const account_object &database_fixture::account_create(
                const string &name,
                const public_key_type &key
        ) {
            return account_create(name, key, key);
        }

        const witness_object &database_fixture::witness_create(
                const string &owner,
                const private_key_type &owner_key,
                const string &url,
                const public_key_type &signing_key,
                const share_type &fee) {
            try {
                witness_update_operation op;
                op.owner = owner;
                op.url = url;
                op.block_signing_key = signing_key;
                op.fee = asset(fee, STEEM_SYMBOL);

                trx.operations.push_back(op);
                trx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
                trx.sign(owner_key, db->get_chain_id());
                trx.validate();
                db->push_transaction(trx, 0);
                trx.operations.clear();
                trx.signatures.clear();

                return db->get_witness(owner);
            }
            FC_CAPTURE_AND_RETHROW((owner)(url))
        }

        void database_fixture::fund(
                const string &account_name,
                const share_type &amount
        ) {
            try {
                transfer(STEEMIT_INIT_MINER_NAME, account_name, amount);

            } FC_CAPTURE_AND_RETHROW((account_name)(amount))
        }

        void database_fixture::fund(
                const string &account_name,
                const asset &amount
        ) {
            try {
                db_plugin->debug_update([=](database &db) {
                    db.modify(db.get_account(account_name), [&](account_object &a) {
                        if (amount.symbol == STEEM_SYMBOL) {
                            a.balance += amount;
                        } else if (amount.symbol == SBD_SYMBOL) {
                            a.sbd_balance += amount;
                        }
                    });

                    db.modify(db.get_dynamic_global_properties(), [&](dynamic_global_property_object &gpo) {
                        if (amount.symbol == STEEM_SYMBOL) {
                            gpo.current_supply += amount;
                        } else if (amount.symbol == SBD_SYMBOL) {
                            gpo.current_sbd_supply += amount;
                        }
                    });

                    if (amount.symbol == SBD_SYMBOL) {
                        const auto &median_feed = db.get_feed_history();
                        if (median_feed.current_median_history.is_null()) {
                            db.modify(median_feed, [&](feed_history_object &f) {
                                f.current_median_history = price(asset(1, SBD_SYMBOL), asset(1, STEEM_SYMBOL));
                            });
                        }
                    }

                    db.update_virtual_supply();
                }, default_skip);
            }
            FC_CAPTURE_AND_RETHROW((account_name)(amount))
        }

        void database_fixture::convert(
                const string &account_name,
                const asset &amount) {
            try {
                const account_object &account = db->get_account(account_name);


                if (amount.symbol == STEEM_SYMBOL) {
                    db->adjust_balance(account, -amount);
                    db->adjust_balance(account, db->to_sbd(amount));
                    db->adjust_supply(-amount);
                    db->adjust_supply(db->to_sbd(amount));
                } else if (amount.symbol == SBD_SYMBOL) {
                    db->adjust_balance(account, -amount);
                    db->adjust_balance(account, db->to_steem(amount));
                    db->adjust_supply(-amount);
                    db->adjust_supply(db->to_steem(amount));
                }
            } FC_CAPTURE_AND_RETHROW((account_name)(amount))
        }

        void database_fixture::transfer(
                const string &from,
                const string &to,
                const share_type &amount) {
            try {
                transfer_operation op;
                op.from = from;
                op.to = to;
                op.amount = amount;

                trx.operations.push_back(op);
                trx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
                trx.validate();
                db->push_transaction(trx, ~0);
                trx.operations.clear();
            } FC_CAPTURE_AND_RETHROW((from)(to)(amount))
        }

        void database_fixture::vest(const string &from, const share_type &amount) {
            try {
                transfer_to_vesting_operation op;
                op.from = from;
                op.to = "";
                op.amount = asset(amount, STEEM_SYMBOL);

                trx.operations.push_back(op);
                trx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
                trx.validate();
                db->push_transaction(trx, ~0);
                trx.operations.clear();
            } FC_CAPTURE_AND_RETHROW((from)(amount))
        }

        void database_fixture::vest(const string &account, const asset &amount) {
            if (amount.symbol != STEEM_SYMBOL) {
                return;
            }

            db_plugin->debug_update([=](database &db) {
                db.modify(db.get_dynamic_global_properties(), [&](dynamic_global_property_object &gpo) {
                    gpo.current_supply += amount;
                });

                db.create_vesting(db.get_account(account), amount);

                db.update_virtual_supply();
            }, default_skip);
        }

        void database_fixture::proxy(const string &account, const string &proxy) {
            try {
                account_witness_proxy_operation op;
                op.account = account;
                op.proxy = proxy;
                trx.operations.push_back(op);
                db->push_transaction(trx, ~0);
                trx.operations.clear();
            } FC_CAPTURE_AND_RETHROW((account)(proxy))
        }

        void database_fixture::set_price_feed(const price &new_price) {
            try {
                for (int i = 1; i < STEEMIT_MAX_WITNESSES; i++) {
                    feed_publish_operation op;
                    op.publisher = STEEMIT_INIT_MINER_NAME + fc::to_string(i);
                    op.exchange_rate = new_price;
                    trx.operations.push_back(op);
                    trx.set_expiration(db->head_block_time() + STEEMIT_MAX_TIME_UNTIL_EXPIRATION);
                    db->push_transaction(trx, ~0);
                    trx.operations.clear();
                }
            } FC_CAPTURE_AND_RETHROW((new_price))

            generate_blocks(STEEMIT_BLOCKS_PER_HOUR);
#ifdef STEEMIT_BUILD_TESTNET
            BOOST_REQUIRE(!db->skip_price_feed_limit_check ||
                          db->get(feed_history_id_type()).current_median_history ==
                          new_price);
#else
            BOOST_REQUIRE(db->get(feed_history_id_type()).current_median_history == new_price);
#endif
        }

        const asset &database_fixture::get_balance(const string &account_name) const {
            return db->get_account(account_name).balance;
        }

        void database_fixture::sign(signed_transaction &trx, const fc::ecc::private_key &key) {
            trx.sign(key, db->get_chain_id());
        }

        vector<operation> database_fixture::get_last_operations(uint32_t num_ops) {
            vector<operation> ops;
            const auto& acc_hist_idx = db->get_index<golos::plugins::account_history::account_history_index>().indices().get<by_id>();
            auto itr = acc_hist_idx.end();
            while (itr != acc_hist_idx.begin() && ops.size() < num_ops) {
                itr--;
                ops.push_back(fc::raw::unpack<golos::chain::operation>(db->get(itr->op).serialized_op));
            }
            return ops;
        }

        void database_fixture::validate_database(void) {
            try {
                db->validate_invariants();
            }
            FC_LOG_AND_RETHROW();
        }

        namespace test {

            bool _push_block(database &db, const signed_block &b, uint32_t skip_flags /* = 0 */ ) {
                return db.push_block(b, skip_flags);
            }

            void _push_transaction(database &db, const signed_transaction &tx, uint32_t skip_flags /* = 0 */ ) {
                try {
                    db.push_transaction(tx, skip_flags);
                } FC_CAPTURE_AND_RETHROW((tx))
            }

        } // golos::chain::test

} } // golos::chain
