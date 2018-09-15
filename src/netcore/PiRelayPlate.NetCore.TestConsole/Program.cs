using System;

namespace PiRelayPlate.NetCore.TestConsole
{
    class Program
    {
        static void Main(string[] args)
        {
            // Loop over the relay stack
            var devicesToProcess = 7;
            var pinsperDevice = 7;

            for (byte devicesId = 0; devicesId < devicesToProcess; devicesId++)
            {
                for (byte pin = 0; pin < pinsperDevice; pin++)
                {
                    Console.WriteLine($"Device: {devicesId}; Pin: {pin}; (cycle)");
                    var result = RelayPlate.SetPinState(devicesId, pin, 1);
                    if (result != 0)
                    {
                        Console.WriteLine($"Device: {devicesId}; Pin: {pin} failed with returned with code {result}");
                        continue;
                    }
                    System.Threading.Thread.Sleep(250);
                    RelayPlate.SetPinState(devicesId, pin, 0);
                }
            }

        }
    }
}
