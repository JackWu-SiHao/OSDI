struct sock *tcp_check_req(struct sock *sk, struct sk_buff *skb,
               struct request_sock *req,
               struct request_sock **prev);

    inet_csk_reqsk_queue_is_full(const struct sock *sk);

    inet_csk_reqsk_queue_hash_add(struct sock *sk, struct request_sock *req,
         unsigned long timeout);

    inet_csk_reqsk_queue_drop(struct sock *sk, struct request_sock *req,
        struct request_sock **prev);

        inet_csk_reqsk_queue_unlink(sk, req, prev);
        inet_csk_reqsk_queue_removed(sk, req);
        reqsk_free(req);

bh_lock_sock(sk);
bh_unlock_sock(sk);
