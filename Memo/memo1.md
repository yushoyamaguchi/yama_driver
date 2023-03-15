# printkの出力
dmesgコマンドで出力を確認できる

# ドライバが保持するデータ
rx/txリングなどのドライバが保持しておくデータは、netdev構造体のプライベートデータとして持っておく。

# 疑問1
baseの場所どうやってみつける？
## 答え
adapter構造体にbarを保持する変数を作る。
<br>
tblを登録して、pci_registerを登録した時点で、probe関数の引数のpci_dev構造体に該当デバイスのデータが入るようになってる。
その情報を引数に適切な関数を呼ぶことでbarも取得できる。

# ToDo
## probe
pci_enable_deviceを呼び出す
<br>
mmioベースアドレス、pioベースアドレスとかを取得して、adapter構造体に格納する
<br>
mmioアドレスの場合は、pci_ioremap_barとかで仮想メモリ空間にマッピングしたもの使う
<br>
rx_init/tx_init
<br>
rx/txのリングバッファはdma_alloc_coherentで用意する。

## statusがupにならない問題
調査する。tx実装してからでも良い。
<br>
- macアドレス割り当てるコマンドは効くか
- ip link set XX up のときのログを見る

### やること
open関数の実装
- netif_start_queue
- netif_carrier_on

## tx
最初はschduleとかは考えなくて良い

## rx
rxは割り込みの実装が終わってから


## rmmod時のエラー
呼び出されてそうな関数をチェック

# 気づき
Linuxは、初期化時にmmio領域を埋めるという処理を一つの関数にまとめず、意味的な処理で分けている。

# 状況0315
ip linkでlinkは見えるが、macアドレスが振られておらず、```ip link set eth0 up``` しても、statusがupにならない。
