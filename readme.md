#サクラエディタ 私用版
　このサクラエディタは、各種パッチを適用して新機能の開発・テストのため本家版からforkしたものです。
また、個人のGitのテスト運用を兼ねています。  
　tags/branchs/trunk等trunk2以外はミラーしていません。

## 本家
　http://sakura-editor.sourceforge.net/

##バイナリの入手
x86版 release tagのページからどうぞ  
http://github.com/mocaskr/sakura/releases/tag/v002.20140611-r3730

## 変更内容
sakura 2.1.1.3 mocaskr_ver 002.20140611-r3730  
 311	コードページ対応  
 636	DiffのSJIS以外に対応  
 637	Grep置換  
 694	SearchNext等のマクロで検索設定を元に戻すオプション  
 713	プロポーショナルフォント  
 716	マルチプロファイル対応  
 717	C/C++インデントの改良  
 726	エンコーディング名による文字コードの設定  
 735	ファイル読み込みでスレッドを使う  
 744	DLLSHAREDATAのサイズチェック  
 738	関数リストマーク追加  
 739	変更行へ移動コマンド追加  
 743	改行置換時次の行を変更状態にしない  
 751	PageUp/Downマクロの修正  
 758	行属性取得マクロ追加  
 759	選択ロック取得マクロ追加  
 760	画面スクロール位置設定・画面行桁数取得マクロ追加  
 761	ポップアップメニュー作成マクロ追加  
 762	Grepで標準出力に出力するコマンドラインオプションを追加  
 765	偶数行の背景色の修正とノート線  
 773	タブ多段・下に表示  
 777	NEL,PS,LSの改行コード表示を変更  
 803	ブックマークの記憶方法の変更  
 805	DLL,EXEの検索パスのセキュリティ設定変更  
 821	上書きモードでの矩形選択入力で選択範囲を削除するオプションを追加  
 826	コンパイルオプションに/NXCOMPAT /SAFESEH /DYNAMICBASEを追加  
 837	フォントスタイルに斜体(仮)と取り消し線を追加  
 829	ファイルパスを省略表示するオプションの追加  
 838	大きいファイルの暫定サポート  
番号と詳細は http://sourceforge.net/p/sakura-editor/patchunicode/ 参照  
