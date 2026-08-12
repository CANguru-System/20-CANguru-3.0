namespace CANguru.GUI.Forms
{
    partial class LokListForm
    {
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.ListView listViewLoks;
        private System.Windows.Forms.ColumnHeader colCount;
        private System.Windows.Forms.ColumnHeader colName;
        private System.Windows.Forms.ColumnHeader colAddress;
        private System.Windows.Forms.ColumnHeader colUid;
        private System.Windows.Forms.ColumnHeader colType;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            listViewLoks = new ListView();
            colCount = new ColumnHeader();
            colName = new ColumnHeader();
            colAddress = new ColumnHeader();
            colUid = new ColumnHeader();
            colType = new ColumnHeader();
            btnSaveLokList = new Button();
            btnOk = new Button();
            SuspendLayout();
            // 
            // listViewLoks
            // 
            listViewLoks.Columns.AddRange(new ColumnHeader[] { colCount, colName, colAddress, colUid, colType });
            listViewLoks.FullRowSelect = true;
            listViewLoks.GridLines = true;
            listViewLoks.Location = new Point(14, 14);
            listViewLoks.Margin = new Padding(4, 3, 4, 3);
            listViewLoks.Name = "listViewLoks";
            listViewLoks.Size = new Size(566, 477);
            listViewLoks.TabIndex = 0;
            listViewLoks.UseCompatibleStateImageBehavior = false;
            listViewLoks.View = View.Details;
            // 
            // colCount
            // 
            colCount.Text = "Nr.";
            colCount.Width = 50;
            // 
            // colName
            // 
            colName.Text = "Name";
            colName.Width = 150;
            // 
            // colAddress
            // 
            colAddress.Text = "Adresse";
            colAddress.Width = 80;
            // 
            // colUid
            // 
            colUid.Text = "UID";
            colUid.Width = 150;
            // 
            // colType
            // 
            colType.Text = "Typ";
            colType.Width = 100;
            // 
            // btnSaveLokList
            // 
            btnSaveLokList.Location = new Point(727, 497);
            btnSaveLokList.Name = "btnSaveLokList";
            btnSaveLokList.Size = new Size(75, 23);
            btnSaveLokList.TabIndex = 1;
            btnSaveLokList.Text = "Speichern";
            btnSaveLokList.UseVisualStyleBackColor = true;
            btnSaveLokList.Click += btnSaveLokList_Click;
            // 
            // btnOk
            // 
            btnOk.Location = new Point(825, 497);
            btnOk.Name = "btnOk";
            btnOk.Size = new Size(75, 23);
            btnOk.TabIndex = 2;
            btnOk.Text = "Schließen";
            btnOk.UseVisualStyleBackColor = true;
            btnOk.Click += btnOk_Click;
            // 
            // LokListForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(596, 532);
            Controls.Add(btnOk);
            Controls.Add(btnSaveLokList);
            Controls.Add(listViewLoks);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            Margin = new Padding(4, 3, 4, 3);
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "LokListForm";
            Text = "Lokliste";
            ResumeLayout(false);
        }
        private Button btnSaveLokList;
        private Button btnOk;
    }
}
