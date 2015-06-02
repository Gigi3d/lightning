/* My example:
 * ./check-commit-sig A-open.pb B-open.pb A-commit-sig.pb B-commit-sig.pb cUBCjrdJu8tfvM7FT8So6aqs6G6bZS1Cax6Rc9rFzYL6nYG4XNEC A-leak-anchor-sigs.pb B-leak-anchor-sigs.pb > A-commit.tx
 * ./check-commit-sig B-open.pb A-open.pb B-commit-sig.pb A-commit-sig.pb cQXhbUnNRsFcdzTQwjbCrud5yVskHTEas7tZPUWoJYNk5htGQrpi B-leak-anchor-sigs.pb A-leak-anchor-sigs.pb > B-commit.tx
 */
#include <ccan/crypto/shachain/shachain.h>
#include <ccan/short_types/short_types.h>
#include <ccan/tal/tal.h>
#include <ccan/opt/opt.h>
#include <ccan/str/hex/hex.h>
#include <ccan/err/err.h>
#include <ccan/read_write_all/read_write_all.h>
#include "lightning.pb-c.h"
#include "anchor.h"
#include "base58.h"
#include "pkt.h"
#include "bitcoin_script.h"
#include "permute_tx.h"
#include "signature.h"
#include "commit_tx.h"
#include "pubkey.h"
#include <openssl/ec.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	const tal_t *ctx = tal_arr(NULL, char, 0);
	OpenChannel *o1, *o2;
	OpenCommitSig *cs1, *cs2;
	struct bitcoin_tx *anchor, *commit;
	struct sha256_double txid;
	u8 *tx_arr;
	size_t *inmap, *outmap;
	struct pubkey pubkey1, pubkey2;
	struct signature sig1, sig2;
	char *tx_hex;

	err_set_progname(argv[0]);

	opt_register_noarg("--help|-h", opt_usage_and_exit,
			   "<open-channel-file1> <open-channel-file2> <commit-sig-1> <commit-sig-2> <commit-key1> <leak-anchor-sigs1> <leak-anchor-sigs2>\n"
			   "Output the commitment transaction if both signatures are valid",
			   "Print this message.");

 	opt_parse(&argc, argv, opt_log_stderr_exit);

	if (argc != 8)
		opt_usage_and_exit(NULL);

	o1 = pkt_from_file(argv[1], PKT__PKT_OPEN)->open;
	o2 = pkt_from_file(argv[2], PKT__PKT_OPEN)->open;
	cs1 = pkt_from_file(argv[3], PKT__PKT_OPEN_COMMIT_SIG)->open_commit_sig;
	cs2 = pkt_from_file(argv[4], PKT__PKT_OPEN_COMMIT_SIG)->open_commit_sig;

	/* Get the transaction ID of the anchor. */
	anchor = anchor_tx_create(ctx, o1, o2, &inmap, &outmap);
	if (!anchor)
		errx(1, "Failed transaction merge");
	anchor_txid(anchor, argv[6], argv[7], inmap, &txid);

	/* Now create THEIR commitment tx. */
	commit = create_commit_tx(ctx, o2, o1, &txid, outmap[0]);

	/* If contributions don't exceed fees, this fails. */
	if (!commit)
		errx(1, "Contributions %llu & %llu vs fees %llu & %llu",
		     (long long)o1->anchor->total,
		     (long long)o2->anchor->total,
		     (long long)o1->commitment_fee,
		     (long long)o2->commitment_fee);

	/* Signatures and pubkeys well-formed? */
	if (!proto_to_signature(cs1->sig, &sig1))
		errx(1, "Invalid commit-sig-1");
	if (!proto_to_signature(cs2->sig, &sig2))
		errx(1, "Invalid commit-sig-2");
	if (!proto_to_pubkey(o1->anchor->pubkey, &pubkey1))
		errx(1, "Invalid anchor-1 key");
	if (!proto_to_pubkey(o2->anchor->pubkey, &pubkey2))
		errx(1, "Invalid anchor-2 key");
	
	/* Their signature must validate correctly. */
	if (!check_2of2_sig(commit, 0, &anchor->output[outmap[0]],
			    &pubkey1, &pubkey2, &sig1, &sig2))
		errx(1, "Signature failed");

	/* Create p2sh input for commit */
	commit->input[0].script = scriptsig_p2sh_2of2(commit, &sig1, &sig2,
						      &pubkey1, &pubkey2);
	commit->input[0].script_length = tal_count(commit->input[0].script);

	/* Print it out in hex. */
	tx_arr = linearize_tx(ctx, commit);
	tx_hex = tal_arr(tx_arr, char, hex_str_size(tal_count(tx_arr)));
	hex_encode(tx_arr, tal_count(tx_arr), tx_hex, tal_count(tx_hex));

	if (!write_all(STDOUT_FILENO, tx_hex, strlen(tx_hex)))
		err(1, "Writing out transaction");

	tal_free(ctx);
	return 0;
}

