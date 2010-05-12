
#ifndef GALERA_BYPASS_WSDB_HPP
#define GALERA_GALERA_WSDB_HPP

#include "wsdb.hpp"
#include "wsrep_api.h"
#include <boost/unordered_map.hpp>

namespace galera
{
    
    class GaleraWsdb : public Wsdb
    {
        // TODO: This can be optimized
        class TrxHash
        {
        public:
            size_t operator()(const wsrep_trx_id_t& key) const
            {
                return (key & 0xffff);
            }
        };

        class Conn
        {
        public:
            Conn(wsrep_conn_id_t conn_id) 
                : 
                conn_id_(conn_id), 
                default_db_(),
                trx_(0)
            { }
            
            Conn(const Conn& other)
                :
                conn_id_(other.conn_id_),
                default_db_(other.default_db_),
                trx_(other.trx_)
            { }
            
            ~Conn() { if (trx_ != 0) trx_->unref(); }
            
            void assign_trx(TrxHandle* trx)
            {
                if (trx_ != 0) trx_->unref();
                trx_ = trx;
            }

            TrxHandle* get_trx() 
            { 
                return trx_; 
            }

            void assing_default_db(const Query& query)
            {
                default_db_ = query;
            }
            
            const Query& get_default_db() const
            {
                return default_db_;
            }

        private:
            void operator=(const Conn&);
            wsrep_conn_id_t conn_id_;
            Query default_db_;
            TrxHandle* trx_;
        };

        typedef boost::unordered_map<wsrep_trx_id_t, TrxHandle*, TrxHash> TrxMap;
        typedef boost::unordered_map<wsrep_conn_id_t, Conn> ConnMap;

    public:
        TrxHandle* get_trx(const wsrep_uuid_t& source_id,
                           wsrep_trx_id_t trx_id, bool create = false);
        TrxHandle* get_conn_query(const wsrep_uuid_t&,
                                  wsrep_conn_id_t conn_id, 
                                  bool create = false);
        // Discard trx handle
        void discard_trx(wsrep_trx_id_t trx_id);
        void discard_conn(wsrep_conn_id_t conn_id);
        
        void append_query(TrxHandle*, const void* query, size_t query_len,
                          time_t, uint32_t);

        void append_row_key(TrxHandle*,
                            const void* dbtable, 
                            size_t dbtable_len,
                            const void* key, 
                            size_t key_len,
                            int action);
        
        void append_conn_query(TrxHandle*, const void* query,
                               size_t query_len);
        void discard_conn_query(wsrep_conn_id_t conn_id);
        
        void set_conn_variable(TrxHandle*, 
                               const void*, size_t,
                               const void*, size_t);
        void set_conn_database(TrxHandle*, const void*, size_t);
        void create_write_set(TrxHandle*, 
                              const void* rbr_data,
                              size_t rbr_data_len);
        std::ostream& operator<<(std::ostream& os) const;
        GaleraWsdb();
        ~GaleraWsdb();
        
    private:
        // Create new trx handle
        TrxHandle* create_trx(const wsrep_uuid_t&, wsrep_trx_id_t trx_id);
        Conn& create_conn(wsrep_conn_id_t conn_id);
        
        TrxMap       trx_map_;
        ConnMap      conn_map_;
        gu::Mutex    mutex_;
    };

}


#endif // GALERA_GALERA_WSDB_HPP