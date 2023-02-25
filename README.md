# USB_Host_Shield_Library_2.0_BTSSP
The code is released under the GNU General Public License.

## Summary
This library is based on the USB Host Shield Library 2.0 with modifications to support Secure Simple Pairing on Bluetooth to use the Microsoft Game Controller (for Xbox One S/Windows MODEL 1708).

## 概要
このライブラリは、USB Host Shield Library 2.0 を元にして、Microsoftゲームコントローラー（Xbox One S/Windows用 MODEL 1708）が繋がるように、ライブラリのBluetoothの部分を Secure Simple Pairing 対応に改造したものです。
USB_Host_Shield_2.0については下記を参照してください。

USB_Host_Shield_2.0 リポジトリ  
<http://github.com/felis/USB_Host_Shield_2.0>  
プロジェクトサイト   
<https://chome.nerpa.tech/arduino_usb_host_shield_projects/>

元のライブラリがSecure Simple Pairing(SSP)に対応する前に、自分で改造したライブラリです。今は、元のライブラリも SSP に対応しています。

このライブラリを作成した経緯は以下のブログ記事にまとめてあります。  
<http://kato-h.cocolog-nifty.com/khweblog/2020/09/post-c93f3c.html>  

ここでは、その自分で改造したライブラリを残しておいて、なおかつ自分の勉強と練習のために、さらに好き勝手に手を加えています。
元のライブラリや前の改造ライブラリを書き変えまくって、かなり大きな変更を加えているので、互換性等はあまりないです。

動作保証はしません。ユーザーサポートもしません。使用は自己責任で。何か問題があっても自分で何とかしてください。

商用のプログラムのコード等は書いたことが無い素人で初心者なので、多分、色々と間違っていると思います。
このライブラリのソースコードを、プロの人やC++の熟練者が見ると「なんでこんなコードの書き方をするんだよ！」と怒りさえ覚える人がいるかもしれません。
もし居ましたら、このライブラリをフォークして、添削や清書をして頂ければと思います。

オープンソースなので、ライセンスの範囲内で自由に使ってやってください。

ライブラリの名前を「USB Host Shield Library 2.0 BTXBOX」から「USB Host Shield Library 2.0 BTSSP」に変更しました。

USB_Host_Shield_Library_2.0_BTSSP リポジトリ  
<https://github.com/HisashiKato/USB_Host_Shield_Library_2.0_BTSSP>  

そして、このライブラリを元に、更に改造したライブラリを作り始めました。

USB_Host_Shield_Library_2.0_BTSSP_b リポジトリ  
<https://github.com/HisashiKato/USB_Host_Shield_Library_2.0_BTSSP_b>
　  

## ハードウェア
* Arduino 及び、その互換機（但し、Arduino の EEPROMライブラリ が使えるボードのみ）  
* USB Host Shield 2.0  
* Bluetooth 4.0 USBアダプタ

   
   

## 使用方法
### Arduino IDE にライブラリを組み込む  
USB Host Library Rev.2.0 BTSSPは、Arduino IDEで使用できます。
    
1. Code の Download ZIP 、または下記URLから、ライブラリのZIPをダウンロードします。  
<https://github.com/HisashiKato/USB_Host_Shield_Library_2.0_BTSSP/archive/master.zip>  
   
2. Arduino IDE を起動して、スケッチ > ライブラリをインクルード > .ZIP形式のライブラリをインストール で、ダウンロードした USB_Host_Shield_Library_2.0_BTSSP-master.zip を指定します。  
   
3. Arduino IDE に、「ライブラリが追加されました。「ライブラリをインクルード」メニューを確認してください。」と表示されれば成功。

4. Arduino IDE で、メニューの スケッチ > ライブラリをインクルード のライブラリの一覧に、USB Host Shield Library 2.0 BTSSP が表示されているのを確認してください。

   
   
   
### ライブラリの使用方法
先ずは、元の USB Host Library Rev.2.0 の使用方法を読んでください。

仕様が大幅に変わりました。具体的な使い方は、スケッチ例を参照してください。

このライブラリのスケッチ例では、Arduino の内部のフラッシュ領域にデータを書き込み読み出すEEPROMライブラリを使用しています。注意してください。具体的にはペアリングに成功したデバイスのBDアドレス(Bluetooth Device Address) 6byte、それに続けてペアリング時に生成されたリンクキー 16byte を、アドレスの0番地から書き込んで、再接続時に読みに行っています。（このフラッシュ領域のデータは、スケッチを書き変えても変更されません）

本家の USB Host Library Rev.2.0 では今のところ出来ないSSPのリンクキーの保存が出来るので、再接続等が確実になっています。

元のライブラリを改造して、ここで最初に公開していたライブラリ内の、Bluetoothに関係する関数では、その関数の内部でArduinoのEEPROMライブラリを使用して、リンクキー等の保存の処理をしていましたが、そのBluetoothの関数の内部にあったリンクキー等の保存の処理を、関数から外に出して、ユーザースケッチ側で処理をするように変更しました。なので、EEPROMライブラリが使えないArduino互換ボードでも、スケッチを工夫してどこかの不揮発性メモリにリンクキー等を保存する仕組みを用意することが出来れば、SSPでの接続が行えるようになると思います。

このライブラリは、BluetoothのSSP(Secure Simple Pairing)接続にだけ対応しています。Arduino Uno (ATmega328P) のようなメモリが貧弱なボードでも使えるように、バイナリの容量を極力減らしたいと思って、従来のPS3コントローラー,Wiiリモコン等との接続や、旧来のPINコードを用いた接続は外しました。SDP(Service Discovery Protocol)も未実装です(今のところSDPは無くても動いてる)。USB有線での接続にも対応していません。それらは元の USB Host Library Rev.2.0 を使用してください。

Bluetooth Classic のみです。 Bluetooth Low Energy (BLE) には対応していません。（元の USB Host Library Rev.2.0 も BLE は未実装です） 

※注意：元のライブラリ "USB Host Library Rev.2.0" を使用するときは、このライブラリを削除（USB_Host_Shield_Library_2.0_BTSSP-masterのフォルダを消去）してください。重複して、衝突します。（衝突しました。）逆に、このライブラリを使用するときは、本家のライブラリを削除してください。めんどくさいですが。
   
   

## 対応コントローラー

### Microsoftゲームコントローラー（Xbox One S/Windows用 MODEL 1708）

コントローラーの基本の情報の受信に加えて、振動用データの送信に対応しました。

ゲームコントローラーのファームウェアが違うと、通信の仕様や送られてくるデータが異なります。過去に動作確認を行ったコントローラーのファームウェアは、ライブラリ製作時の 4.8.1923.0 です。使用するコントローラーのファームウェアを確認してください。

ファームウェアを最新の "5.13.3143.0" にアップデートをしたら、そのコントローラーが、このライブラリで使えなくなりました。5.13.3143.0 で、Bluetooth Classic から Bluetooth Low Energy (BLE) に変更されたためです。このライブラリは BLE 未実装です。

ファームウェアをダウングレード(ロールバック)する方法を探しました。古い 3.1.1221.0 に戻す方法は見つかりました。

"My Xbox controller has connection issues after the last update"
https://support.xbox.com/en-US/help/hardware-network/accessories/controller-firmware-reversion

しかし、4.8.1923.0 に戻す方法は見つかりませんでした。

そこで、3.1.1221.0 のボタンの読み取りを追加しました。3.1.1221.0 では動作チェックをしましたが、4.8.1923.0 はファームフェアを戻せないため動作チェックをしていません。

### Nintendo Switch Pro コントローラー

簡易版です。サブコマンドの送信を組み込んだので、振動を暫定的に入れてみました。ジャイロは対応するか未定です。

### SONY DUALSHOCK 4 コントローラー

オリジナルの USB Host Library 2.0 の、DUALSHOCK 4 の関数群を移植しました。

### Bluetooth キーボード,マウス

私が所持しているBluetoothキーボードとBluetoothマウスで、SSPでのペアリングと、ペアリング後の接続と入力が出来ました。最近、セキュリティのリスクを減らすために、オリジナルのライブラリで使用している4ケタの固定のPINコードを使う古いペアリングのモードではペアリングが働かない（ボンディングが行われない）Bluetoothキーボードがあるみたいです。
そのようなキーボードやマウスは、このライブラリなら使用できる可能性があります。

## 参考にさせて頂いたプロジェクトやWEBサイト
<https://github.com/felis/USB_Host_Shield_2.0>：USB_Host_Shield_2.0 の本家

<https://www.silex.jp/blog/wireless/>：サイレックス・テクノロジー株式会社 のblog、Wireless・のおと（Bluetoothの基本）

<http://www.yts.rdy.jp/pic/GB002/GB002.html>：YTSさんの、YTSのホームページ（SSPの詳細等）

<https://hackaday.io/project/170365-blueretro>：Jacques Gagnon 氏の"BlueRetro"（とても参考になりました！！！）

<https://github.com/atar-axis/xpadneo>：Advanced Linux Driver for Xbox One Wireless Gamepad

<https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering>：Nintendo_Switch_Reverse_Engineering

その他、思いついたら追加します。
こうして作ることが出来たのも、みなさんのおかげです。本当にありがとうございます。
