# AviUtl プラグイン - シーン簡単選択

シーンを簡単に切り替えられるようにします。
[最新バージョンをダウンロード](../../releases/latest/)

## 導入方法

以下のファイルを AviUtl の Plugins フォルダに入れてください。
* SelectScene.auf
* SelectScene.ini
* SelectScene (フォルダ) (ボイスが必要な場合のみ)

## 使用方法

1. メニューの「表示」→「シーン簡単選択の表示」を選択してウィンドウを表示します。
2. ウィンドウに並んでいるボタンを押してシーンを切り替えます。

## 設定方法

1. ウィンドウを右クリックしてコンテキストメニューを表示します。
2. 「設定」を選択します。
3. ダイアログが表示されるので行数などを設定します。<br>

* ```レイアウトモード```
	* ```垂直方向``` 縦に ```行数``` 分だけ並んだあと、次の列に改行します。
	* ```水平方向``` 横に ```列数``` 分だけ並んだあと、次の行に改行します。
* ```シーン数``` 表示するシーンの数 (ボタンの数) を指定します。
* ```ボイス``` 再生する wav ファイルを番号で指定します。スピンボタンで変更したときプレビューが再生されます。

## ボイス

SelectScene フォルダ内の wav ファイルが再生されます。

* 0 ボイスなし
* 1 青山龍星（ノーマル）シーンを変更しました
* 2 波音リツ（ノーマル）シーンを変更しました
* 3 九州そら（ささやき）シーン、変更しちゃいました
* 4 青山龍星（ノーマル）お嬢様、っシーンの変更が完了致しました
* 5 波音リツ（ノーマル）シーン変えといたぞっブタヤローオ
* 6 九州そら（ささやき）ばああか、ざああこ
* 7 ~ 10 空き

## 更新履歴

* 1.1.0 - 2022/08/01 シーンの数を制限できるように修正
* 1.0.0 - 2022/07/21 初版

## 動作確認

* (必須) AviUtl 1.10 & 拡張編集 0.92 http://spring-fragrance.mints.ne.jp/aviutl/
* (共存確認) patch.aul r41 https://scrapbox.io/ePi5131/patch.aul

## クレジット

* Microsoft Research Detours Package https://github.com/microsoft/Detours
* aviutl_exedit_sdk https://github.com/ePi5131/aviutl_exedit_sdk
* Common Library https://github.com/hebiiro/Common-Library
* VOICEVOX (青山龍星、波音リツ、九州そら) https://voicevox.hiroshiba.jp/

## 作成者情報
 
* 作成者 - 蛇色 (へびいろ)
* GitHub - https://github.com/hebiiro
* Twitter - https://twitter.com/io_hebiiro

## 免責事項

この作成物および同梱物を使用したことによって生じたすべての障害・損害・不具合等に関しては、私と私の関係者および私の所属するいかなる団体・組織とも、一切の責任を負いません。各自の責任においてご使用ください。
