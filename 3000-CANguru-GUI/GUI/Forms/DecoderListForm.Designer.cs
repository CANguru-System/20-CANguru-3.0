partial class DecoderListForm
{
    private System.ComponentModel.IContainer components = null;
    private ListView lvDecoders;
    private ColumnHeader colType;
    private ColumnHeader colMac;
    private ColumnHeader colId;
    private ColumnHeader colUID;
    private ColumnHeader colDetails;

    protected override void Dispose(bool disposing)
    {
        if (disposing && (components != null))
            components.Dispose();
        base.Dispose(disposing);
    }

    private void InitializeComponent()
    {
        lvDecoders = new ListView();
        colType = new ColumnHeader();
        colMac = new ColumnHeader();
        colId = new ColumnHeader();
        colUID = new ColumnHeader();
        colDetails = new ColumnHeader();
        btnSaveDecoderList = new Button();
        btnOK = new Button();
        SuspendLayout();
        // 
        // lvDecoders
        // 
        lvDecoders.Columns.AddRange(new ColumnHeader[] { colType, colMac, colId, colUID, colDetails });
        lvDecoders.Font = new Font("Consolas", 10F);
        lvDecoders.FullRowSelect = true;
        lvDecoders.GridLines = true;
        lvDecoders.Location = new Point(10, 10);
        lvDecoders.Name = "lvDecoders";
        lvDecoders.Size = new Size(830, 401);
        lvDecoders.TabIndex = 0;
        lvDecoders.UseCompatibleStateImageBehavior = false;
        lvDecoders.View = View.Details;
        // 
        // colType
        // 
        colType.Text = "Typ";
        colType.Width = 80;
        // 
        // colMac
        // 
        colMac.Text = "MAC-Adresse";
        colMac.Width = 150;
        // 
        // colId
        // 
        colId.Text = "ID";
        colId.Width = 40;
        // 
        // colUID
        // 
        colUID.Text = "UID";
        colUID.Width = 50;
        // 
        // colDetails
        // 
        colDetails.Text = "Details";
        colDetails.Width = 500;
        // 
        // btnSaveDecoderList
        // 
        btnSaveDecoderList.Location = new Point(550, 432);
        btnSaveDecoderList.Name = "btnSaveDecoderList";
        btnSaveDecoderList.Size = new Size(75, 23);
        btnSaveDecoderList.TabIndex = 1;
        btnSaveDecoderList.Text = "Speichern";
        btnSaveDecoderList.UseVisualStyleBackColor = true;
        btnSaveDecoderList.Click += btnSaveDecoderList_Click;
        // 
        // btnOK
        // 
        btnOK.Location = new Point(631, 432);
        btnOK.Name = "btnOK";
        btnOK.Size = new Size(75, 23);
        btnOK.TabIndex = 2;
        btnOK.Text = "Schließen";
        btnOK.UseVisualStyleBackColor = true;
        btnOK.Click += btnOK_Click;
        // 
        // DecoderListForm
        // 
        ClientSize = new Size(850, 470);
        Controls.Add(btnOK);
        Controls.Add(btnSaveDecoderList);
        Controls.Add(lvDecoders);
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        Name = "DecoderListForm";
        Text = "Decoder-Liste";
        ResumeLayout(false);

    }

    private Button btnSaveDecoderList;
    private Button btnOK;
}
