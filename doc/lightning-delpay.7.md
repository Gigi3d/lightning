lightning-delpay -- Command for removing a completed or failed payment
============================================================

SYNOPSIS
--------

**delpay** *payment\_hash* \[status\]

DESCRIPTION
-----------

The **delpay** RPC command removes a payment as given in **listsendpays** or **listpays** with a complete or failed
status. However, the command doesn't permit to remove a pending payment.

- *payment\_hash*: Rapresents the unique identifier of a payment. To find it, you can run **listpays** or **listsendpays**;
- *status* is the expected status of the payment. It can be *complete* or *failed*. 
Only delete if the payment status matches. If not specified, defaults to *complete*.

EXAMPLE JSON REQUEST
------------
```json
{
  "id": 82,
  "method": "delpay",
  "params": {
    "payment_hash": "4fa2f1b001067ec06d7f95b8695b8acd9ef04c1b4d1110e3b94e1fa0687bb1e0",
    "status": "complete"
  }
}
```

RETURN VALUE
------------

On success, the command will return a payment object, such as the **listsendpays**. In addition, if the payment is a MPP (Multi part payment) the command return a list of 
payments; a payment object for each partid.

On failure, an error is returned and any payment is deleted. If the lightning process fails before responding, the
caller should use lightning-listsentpays(7) or lightning-listpays(7) to query whether this payment was deleted or not.

The following error codes may occur:

- -32602: Some parameter missed or some parameter is malformed;
- 211: Payment with payment\_hash have a wrong status. To check the correct status run the command **paystatus**;
- 208: Payment with payment\_hash not found.

EXAMPLE JSON RESPONSE
-----
```json
{
   "payments": [
      {
         "id": 2,
         "payment_hash": "8dfd6538eeb33811c9114a75f792a143728d7f05643f38c3d574d3097e8910c0",
         "destination": "0219f8900ee78a89f050c24d8b69492954f9fdbabed753710845eb75d3a75a5880",
         "msatoshi": 1000,
         "amount_msat": "1000msat",
         "msatoshi_sent": 1000,
         "amount_sent_msat": "1000msat",
         "created_at": 1596224858,
         "status": "complete",
         "payment_preimage": "35bd4e2b481a1a84a22215b5372672cf81460a671816960ddb206464359e1822",
         "bolt11": "lntb10n1p0jga20pp53h7k2w8wkvuprjg3ff6l0y4pgdeg6lc9vsln3s74wnfsjl5fzrqqdqdw3jhxazldahx2xqyjw5qcqp2sp5wut5jnhr6n7jd5747ky2g5flmw7hgx9yjnqzu60ps2jf6f7tc0us9qy9qsqu2a0k37nckl62005p69xavlkydkvhnypk4dphffy4x09zltwh9437ad7xkl83tefdarzhu5t30ju5s56wlrg97qkx404pq3srfc425cq3ke9af"
      }
   ]
}

```


AUTHOR
------

Vincenzo Palazzo <<vincenzo.palazzo@protonmail.com>> is mainly responsible.

SEE ALSO
--------

lightning-listpays(7), lightning-listsendpays(7), lightning-paystatus(7).

RESOURCES
---------

Main web site: <https://github.com/ElementsProject/lightning>