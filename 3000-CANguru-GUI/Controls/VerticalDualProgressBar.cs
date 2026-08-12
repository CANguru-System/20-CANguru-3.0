using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.ComponentModel;
namespace CANguru.Controls
{
    [DesignTimeVisible(false)] // Designer egal, wir nutzen es per Code
    public class VerticalDualProgressBar : Control
    {
        [DefaultValue(0)]
        public int CurrentValue { get; set; } = 0;

        [DefaultValue(100)]
        public int CurrentMaximum { get; set; } = 100;

        [DefaultValue(typeof(Color), "LimeGreen")]
        public Color CurrentColor { get; set; } = Color.LimeGreen;

        [DefaultValue("")]
        public string CurrentText { get; set; } = "";

        [DefaultValue(0)]
        public int TotalValue { get; set; } = 0;

        [DefaultValue(100)]
        public int TotalMaximum { get; set; } = 100;


        [DefaultValue(typeof(Color), "DodgerBlue")]
        public Color TotalColor { get; set; } = Color.DodgerBlue;

        [DefaultValue(typeof(Color), "Black")]
        public Color DividerColor { get; set; } = Color.Black;

        [DefaultValue(typeof(Color), "Black")]
        public Color TextColor { get; set; } = Color.Black;

        [DefaultValue("")]
        public string TotalText { get; set; } = "";

        [DefaultValue(false)]
        public bool SingleBarMode { get; set; } = false;

        public VerticalDualProgressBar()
        {
            DoubleBuffered = true;
            ResizeRedraw = true;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            int width = this.Width;
            int height = this.Height;

            // -------------------------
            // SINGLE BAR MODE
            // -------------------------
            if (SingleBarMode)
            {
                float p = (float)CurrentValue / Math.Max(1, CurrentMaximum);
                int h = (int)(p * height);

                Rectangle rect = new Rectangle(
                    0,
                    height - h,
                    width - 1,
                    h
                );

                using (var b = new SolidBrush(CurrentColor))
                    e.Graphics.FillRectangle(b, rect);

                // Rahmen
                e.Graphics.DrawRectangle(
                    Pens.Black,
                    0,
                    0,
                    width - 1,
                    height - 1
                );

                // Text
                string text = string.IsNullOrEmpty(CurrentText)
                    ? $"{(int)(p * 100)}%"
                    : CurrentText;

                TextRenderer.DrawText(
                    e.Graphics,
                    text,
                    this.Font,
                    new Rectangle(0, 0, width, height),
                    TextColor,
                    TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter
                );

                return; // WICHTIG: Dual-Mode nicht weiterzeichnen
            }

            // -------------------------
            // DUAL MODE
            // -------------------------
            int half = width / 2;

            // -------------------------
            // LINKER BALKEN
            // -------------------------
            float p1 = (float)CurrentValue / Math.Max(1, CurrentMaximum);
            int h1 = (int)(p1 * height);

            Rectangle rectLeft = new Rectangle(
                0,
                height - h1,
                half - 1,
                h1
            );

            using (var b = new SolidBrush(CurrentColor))
                e.Graphics.FillRectangle(b, rectLeft);

            e.Graphics.DrawRectangle(
                Pens.Black,
                0,
                0,
                half - 1,
                height - 1
            );

            string text1 = string.IsNullOrEmpty(CurrentText)
                ? $"{(int)(p1 * 100)}%"
                : CurrentText;

            TextRenderer.DrawText(
                e.Graphics,
                text1,
                this.Font,
                new Rectangle(0, 0, half - 1, height),
                TextColor,
                TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter
            );

            // -------------------------
            // TRENNLINIE
            // -------------------------
            using (var p = new Pen(DividerColor, 2))
                e.Graphics.DrawLine(p, half, 0, half, height);

            // -------------------------
            // RECHTER BALKEN
            // -------------------------
            float p2 = (float)TotalValue / Math.Max(1, TotalMaximum);
            int h2 = (int)(p2 * height);

            Rectangle rectRight = new Rectangle(
                half + 1,
                height - h2,
                half - 2,
                h2
            );

            using (var b = new SolidBrush(TotalColor))
                e.Graphics.FillRectangle(b, rectRight);

            e.Graphics.DrawRectangle(
                Pens.Black,
                half + 1,
                0,
                half - 2,
                height - 1
            );

            string text2 = string.IsNullOrEmpty(TotalText)
                ? $"{(int)(p2 * 100)}%"
                : TotalText;

            TextRenderer.DrawText(
                e.Graphics,
                text2,
                this.Font,
                new Rectangle(half + 1, 0, half - 2, height),
                TextColor,
                TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter
            );
        }
    }
}

