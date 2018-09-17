using System;

namespace PiRelayPlate.NetCore.TestConsole
{
    class Program
    {
        static void Main(string[] args)
        {
            // Loop over the relay stack
            const int devicesToProcess = 7;
            const int pinsPerDevice = 7;

            for (byte devicesId = 0; devicesId < devicesToProcess; devicesId++)
            {
                for (byte pin = 0; pin < pinsPerDevice; pin++)
                {
                    Console.WriteLine($"Device: {devicesId}; Pin: {pin}; (cycle)");
                    var result = RelayPlate.SetPinState(devicesId, pin, 1);
                    if (result != true)
                    {
                        Console.WriteLine($"Device: {devicesId}; Pin: {pin} failed");
                        continue;
                    }
                    System.Threading.Thread.Sleep(250);
                    RelayPlate.SetPinState(devicesId, pin, 0);
                }
            }

        }
    }
}
