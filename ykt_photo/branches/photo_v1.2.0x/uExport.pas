unit uExport;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, GridsEh, DBGridEh, ComCtrls, DB, ADODB,
  DBCtrls, Mask, DBCtrlsEh, DBLookupEh,INIFiles,jpeg;

type
  TfrmExport = class(TForm)
    pnlTop: TPanel;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    chkCust: TCheckBox;
    pnl1: TPanel;
    mmoSql: TMemo;
    Label7: TLabel;
    pnl2: TPanel;
    Label8: TLabel;
    lstError: TListBox;
    btnQuery: TButton;
    btnExport: TButton;
    btnExit: TButton;
    pnl3: TPanel;
    dbgrdhData: TDBGridEh;
    Label9: TLabel;
    edtStuEmpNo: TEdit;
    edtName: TEdit;
    dtpBegin: TDateTimePicker;
    dtpEnd: TDateTimePicker;
    cbbDept: TDBLookupComboboxEh;
    cbbType: TDBLookupComboboxEh;
    cbbSpec: TDBLookupComboboxEh;
    ProgressBar1: TProgressBar;
    cbbArea: TDBLookupComboboxEh;
    Label10: TLabel;
    qryType: TADOQuery;
    dsType: TDataSource;
    dsDept: TDataSource;
    qryDept: TADOQuery;
    qryArea: TADOQuery;
    dsArea: TDataSource;
    qrySpec: TADOQuery;
    dsSpec: TDataSource;
    dsQuery: TDataSource;
    qryQuery: TADOQuery;
    SaveDialog1: TSaveDialog;
    Label11: TLabel;
    procedure FormShow(Sender: TObject);
    procedure btnQueryClick(Sender: TObject);
    procedure btnExitClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure btnExportClick(Sender: TObject);
  private
    { Private declarations }
    TempStringList: TStrings;
    procedure fillCbbData();
    procedure queryData();
  public
    { Public declarations }
  end;

var
  frmExport: TfrmExport;

implementation

uses Udm, uCommon;

{$R *.dfm}

procedure TfrmExport.FormShow(Sender: TObject);
begin
  closeQuery;
  fillCbbData();
  dtpBegin.Date:=Date()-365*5;
  dtpEnd.Date:=Date();
  TempStringList:=TStringList.Create;
end;

procedure TfrmExport.fillCbbData;
var
  deptSql:string;
  specSql:string;
  typeSql:string;
  areaSql:string;
begin
  //deptSql:='select '+deptcode+','+deptName+' from '+tbldept;
  //specSql:='select '+specCode+','+specName+' from '+tblspec;
  //typeSql:='select '+typeNo+','+typeName+' from '+tblCutType;
  //areaSql:='select '+areaNo+','+areaName+' from '+tblArea+' where '+areaFather+'=1';

  getFillQuerySql(deptSql,specSql,typeSql,areaSql);
  qryDept.Close;
  qryDept.SQL.Clear;
  qryDept.SQL.Add(deptSql);
  qryDept.Open;
  cbbDept.ListField:=deptName;
  cbbDept.KeyField:=deptCode;

  qryType.Close;
  qryType.SQL.Clear;
  qryType.SQL.Add(typeSql);
  qryType.Open;
  cbbType.KeyField:=typeNo;
  cbbType.ListField:=typeName;

  qrySpec.Close;
  qrySpec.SQL.Clear;
  qrySpec.SQL.Add(specSql);
  qrySpec.Open;
  cbbSpec.KeyField:=specCode;
  cbbSpec.ListField:=specName;

  qryArea.Close;
  qryArea.SQL.Clear;
  qryArea.SQL.Add(areaSql);
  qryArea.Open;
  cbbArea.KeyField:=areaNo;
  cbbArea.ListField:=areaName;
end;

procedure TfrmExport.queryData;
var
  queryIni:TIniFile;
  sqlStr:string;
begin
  if chkCust.Checked=True then
  begin
    sqlStr:=mmoSql.Text;
  end
  else
  begin
    queryIni := nil;
    if FileExists(apppath+'photoquery.ini') = false then
    begin
      Application.MessageBox('系统配置文件已经被破坏，请与系统管理员联系！',
        '系统错误！', mb_ok + mb_iconerror);
      Application.Terminate;
    end;
    try
      queryIni := TIniFile.Create(apppath+'photoquery.ini');
      sqlStr := queryIni.ReadString('photoquery','photoquerysql','');
    finally
      queryIni.Destroy;
    end;
    sqlStr:=sqlStr+' and photo.'+pPhotoDate+'>='+#39+formatdatetime('yyyymmdd',dtpBegin.Date)+#39;
    sqlStr:=sqlStr+' and photo.'+pPhotoDate+'<='+#39+formatdatetime('yyyymmdd',dtpEnd.Date)+#39;
    if edtStuEmpNo.Text<>'' then
      sqlStr:=sqlStr+' and cust.'+stuempNo+'='+#39+edtStuEmpNo.Text+#39;
    if edtName.Text<>'' then
      sqlStr:=sqlStr+' and cust.'+custName+' like '+#39+'%'+edtname.Text+'%'+#39;
    if cbbArea.Text<>'' then
      sqlStr:=sqlStr+' and cust.'+custArea+'='+inttostr(cbbArea.KeyValue);
    if cbbDept.Text<>'' then
      sqlStr:=sqlStr+' and cust.'+custDeptNo+'='+#39+cbbdept.KeyValue+#39;
    if cbbType.Text<>'' then
      sqlStr:=sqlStr+' and cust.'+custType+'='+inttostr(cbbType.KeyValue);
    if cbbSpec.Text<>'' then
      sqlStr:=sqlStr+' and cust.'+custSpecNo+'='+#39+cbbspec.KeyValue+#39;
  end;
  dbgrdhData.Columns[0].FieldName:=custId;
  dbgrdhData.Columns[1].FieldName:=stuempNo;
  dbgrdhData.Columns[2].FieldName:=custName;
  dbgrdhData.Columns[3].FieldName:=deptName;
  dbgrdhData.Columns[4].FieldName:=specName;
  dbgrdhData.Columns[5].FieldName:=typeName;
  dbgrdhData.Columns[6].FieldName:=classNo;
  qryQuery.Close;
  qryQuery.SQL.Clear;
  qryQuery.SQL.Add(sqlStr);
  //qryQuery.SQL.SaveToFile('123.txt');
  qryQuery.Open;
  if not qryQuery.IsEmpty then
  begin
    qryQuery.First;
    while not qryQuery.Eof do
    begin
      //if qryQuery.fieldbyname(custId).AsString<>'' then
      TempStringList.Add(qryQuery.fieldbyname(custId).AsString);
      qryQuery.Next;
    end;
  end;
end;

procedure TfrmExport.btnQueryClick(Sender: TObject);
begin
  if dtpBegin.Date>dtpEnd.Date then
  begin
    showmessage('你选择的开始日期大于结束日期，请从新选择！');
    exit;
  end;
  TempStringList.Clear;
  queryData();
end;

procedure TfrmExport.btnExitClick(Sender: TObject);
begin
  close();
end;

procedure TfrmExport.FormDestroy(Sender: TObject);
begin
  TempStringList.Destroy;
end;

procedure TfrmExport.btnExportClick(Sender: TObject);
var
  i: integer;
  M1: TMemoryStream;
  Fjpg: TJpegImage;
  picPath:string;
  photoName: string;
  tmpQuery:TADOQuery;
begin
  if TempStringList.Count <= 0 then
  begin
    ShowMessage('请先查询数据,然后再导出！');
    Exit;
  end;
  if ExportData(SaveDialog1,dbgrdhData)=False then
    Exit;

  picPath:=SaveDialog1.Title;
  picpath:=picpath+formatdatetime('yyyymmdd',Date());
  if not directoryExists(picpath) then
    if not CreateDir(picpath) then
      raise Exception.Create('不能创建文件夹：'+picpath);

  qryQuery.First;
  ProgressBar1.Min := 0;
  ProgressBar1.Max := TempStringList.Count;
  for i := 0 to TempStringList.Count - 1 do
  begin
    //**************************************************************************
    tmpQuery:=nil;
    M1:=nil;
    Fjpg:=nil;
    try
      tmpQuery:=TADOQuery.Create(nil);
      tmpQuery.Connection:=frmdm.conn;
      tmpQuery.Close;
      tmpQuery.SQL.Clear;
      tmpQuery.SQL.Add('select p.'+custId+',c.'+stuempNo+',p.'+pPhoto+' from '+tblPhoto);
      tmpQuery.SQL.Add(' p left join '+tblCust+' c on p.'+custId+'=c.'+custId);
      tmpQuery.SQL.Add(' where p.'+custId+'='+#39+TempStringList.Strings[i]+#39);
      tmpQuery.Open;
      if not tmpQuery.IsEmpty then
      begin
        photoName := tmpQuery.FieldByName(stuempNo).AsString;
        Fjpg := TJpegImage.Create;
        M1 := TMemoryStream.Create;
        M1.Clear;
        if TBlobField(tmpQuery.FieldByName(pPhoto)).AsString <> null then
          TBlobField(tmpQuery.FieldByName(pPhoto)).SaveToStream(M1);
        M1.Position := 0;
        if M1.Size > 10 then
        begin
          FJpg.LoadFromStream(M1);
          FJpg.SaveToFile(picpath + '\' + photoName + '.jpg');
        end
        else
        begin
          lstError.items.add('学/工号：' + photoName + '信息导出失败！');
          Continue;
        end;
      end;
    finally
      M1.Free;
      FJpg.Free;
      tmpQuery.Destroy;
    end;
    qryQuery.Next;
    ProgressBar1.Position := i + 1;
  end;
  ShowMessage('完成导出人员照片信息，文件存放位置--'+picpath);
  ProgressBar1.Position := 0;
end;

end.
