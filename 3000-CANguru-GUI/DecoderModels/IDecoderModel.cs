namespace CANguru.DecoderModels
{
    public interface IDecoderModel
    {
        string Name { get; }

        // Wird aufgerufen, wenn die UI Werte zurückspeichert
        void ApplyChanges();

        // Wird aufgerufen, wenn die UI initialisiert wird
        void LoadValues();
    }
}
