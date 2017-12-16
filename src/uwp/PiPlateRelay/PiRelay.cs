using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.Gpio;
using Windows.Devices.Spi;

namespace PiPlateRelay
{
    public enum PiRelayCommand : byte
    {
        SystemAddress = 0x0,
        SystemId = 0x1,
        SystemHardwareRevision = 0x02,
        SystemFirmwreRevision = 0x03,
        SystemReset = 0x0F,
        RelayOn = 0x10,
        RelayOff = 0x11,
        RelayToggle = 0x12,
        RelayAll = 0x13,
        RelayState = 0x14,
        LedSet = 0x60,
        LedClear = 0x61,
        LedToggle = 0x62,
    }

    public struct PiRelayInfo
    {
        public string HardwareId;
        public double FirmwareRevision;
        public double HardwareRevision;
    }

    public static class PiRelay
    {
        private static bool _isInitilized;

        public static IReadOnlyDictionary<byte, PiRelayInfo> RelaysAvailable { get; private set; }

        private static readonly Dictionary<byte, PiRelayInfo> Relays = new Dictionary<byte, PiRelayInfo>();

        private const byte GpioBaseAddr = 24;

        private static GpioController _gpio;

        private static GpioPin _ppFrame;

        private static GpioPin _ppInt;

        private static SpiDevice _spi;

        public static async Task InititlizeAsync()
        {
            RelaysAvailable = Relays;

            InitGpio();
            await InitSpiAsync();
            InitRelays();
        }

        /// <summary>
        /// Setup the Gpio for the RP. Pin 25 and 22 are used
        /// </summary>
        private static void InitGpio()
        {
            if (_gpio == null)
            {
                _gpio = GpioController.GetDefault();
                if (_gpio == null)
                {
                    throw new PiRelayException("There is no GPIO controller on this device.");
                }
            }
            if (_ppFrame == null)
            {
                _ppFrame = _gpio.OpenPin(25, 0);
                _ppFrame.SetDriveMode(GpioPinDriveMode.Output);
            }
            if (_ppInt == null)
            {
                _ppInt = _gpio.OpenPin(22, GpioSharingMode.SharedReadOnly);
            }
        }

        /// <summary>
        /// Initilize the Spi per the board specifiction
        /// </summary>
        private static async Task InitSpiAsync()
        {
            if (_spi == null)
            {
                var spiSettings = new SpiConnectionSettings(1)
                {
                    ClockFrequency = 500000,
                    Mode = 0,
                    SharingMode = SpiSharingMode.Shared
                };
                var settings = spiSettings;
                var deviceInformationCollection = await DeviceInformation.FindAllAsync(SpiDevice.GetDeviceSelector("SPI0"));
                if (deviceInformationCollection.Count == 0)
                {
                    throw new PiRelayException("There is no SPI bus 0 on this device.");
                }

                _spi = SpiDevice.FromIdAsync(deviceInformationCollection[0].Id, settings).AsTask<SpiDevice>().ConfigureAwait(false).GetAwaiter().GetResult();
                if (_spi == null)
                {
                    throw new PiRelayException("There is no SPI bus 0 on this device.");
                }
                
            }
        }

        /// <summary>
        /// Detect the Relay boards. There could up to 8 boards together
        /// </summary>
        /// <returns></returns>
        private static void InitRelays()
        {
            // For some reason, the first command needs to be sent over the SPI
            // There most likely is something I can send after I init the SPI
            // but this is just a simple hack that works.
            SendCommand(0, PiRelayCommand.SystemAddress, 0, 0, 1, true);

            for (byte address = 0; address < 8; address++)
            {
                var cmd = SendCommand(address, PiRelayCommand.SystemAddress, 0, 0, 1, true);
                //cmd = await SendCommand(address, PiRelayCommand.SystemId, 0, 0, 20, true);
                var returnedAddress = cmd[0] - GpioBaseAddr;
                if (returnedAddress != address) continue;

                var id = SendCommand(address, PiRelayCommand.SystemId, 0, 0, 20, true);
                var idStr = Encoding.ASCII.GetString(id);
                var state = new PiRelayInfo()
                {
                    FirmwareRevision = GetFirmwreRevision(address, true),
                    HardwareRevision = GetHardwareRevision(address, true),
                    HardwareId = idStr
                };
                // Reset the states to off
                for(byte relay = 1; relay <= 7; relay++)
                {
                    RelayOffAsync(address, relay, true);
                }
                Relays.Add(address, state);
            }
            _isInitilized = true;
        }

        public static byte[] SendCommand(byte address, PiRelayCommand piRelayCommand, byte param1, byte param2, int bytes2Return, bool ignoreChecks = false)
        {
            // Checks to ensure that the library has been initlized and the relay
            // address is available.
            if (!ignoreChecks)
            {
                if (!_isInitilized) throw new PiRelayException("PiRelay Library has not been Initilized");
                if (!Relays.ContainsKey(address)) throw new PiRelayException($"PiRelay Address {address} not available");
            }

            var addr = address + GpioBaseAddr;

            var arg = new byte[4];
            var resp = new byte[bytes2Return];
            arg[0] = (byte)(addr);
            arg[1] = (byte)piRelayCommand;
            arg[2] = param1;
            arg[3] = param2;

            _ppFrame.Write(GpioPinValue.High);
            _spi.ConnectionSettings.DataBitLength = 60;
            _spi.ConnectionSettings.ClockFrequency = 300000;
            _spi.Write(arg);

            if (bytes2Return > 0)
            {
                _spi.ConnectionSettings.ClockFrequency = 500000;
                _spi.ConnectionSettings.DataBitLength = 20;
                for (var i = 0; i < bytes2Return; i++)
                {
                    byte[] r = new byte[1];
                    _spi.TransferFullDuplex(r, r);
                    resp[i] = r[0];
                }
            }

            _ppFrame.Write(GpioPinValue.Low);


            return resp;
        }

        

        public static string GetId(byte address, bool ignoreChecks)
        {
            var result = SendCommand(address, PiRelayCommand.SystemId, 0, 0, 20, ignoreChecks);
            return Encoding.ASCII.GetString(result);
        }

        public static double GetHardwareRevision(byte address, bool ignoreChecks = false)
        {
            var resp = SendCommand(address, PiRelayCommand.SystemHardwareRevision, 0, 0, 1, ignoreChecks);
            var rev = resp[0];
            var whole = rev >> 4;
            var point = rev & 0x0F;
            return whole + point / 10.0;
        }

        public static double GetFirmwreRevision(byte address, bool ignoreChecks = false)
        {
            var resp = SendCommand(address, PiRelayCommand.SystemHardwareRevision, 0, 0, 1, ignoreChecks);
            var rev = resp[0];
            var whole = rev >> 4;
            var point = rev & 0x0F;
            return whole + point / 10.0;
        }

        //#==============================================================================#	
        //# LED Functions	                                                               #
        //#==============================================================================#   
        public static void LedSet(byte address, bool ignoreChecks = false)
        {
            SendCommand(address, PiRelayCommand.LedSet, 0, 0, 0, ignoreChecks);
        }

        public static void LedClearAsync(byte address, bool ignoreChecks = false)
        {
            SendCommand(address, PiRelayCommand.LedClear, 0, 0, 0, ignoreChecks);
        }

        public static void ToggleLedAsync(byte address, bool ignoreChecks = false)
        {
            SendCommand(address, PiRelayCommand.LedToggle, 0, 0, 0, ignoreChecks);
        }

        //#==============================================================================#	
        //# PiRelay Functions	                                                               #
        //#==============================================================================#   

        public static void RelayOnAsync(byte address, byte relay, bool ignoreChecks = false)
        {
            SendCommand(address, PiRelayCommand.RelayOn, relay, 0, 0, ignoreChecks);
        }

        public static void RelayOffAsync(byte address, byte relay, bool ignoreChecks = false)
        {
            SendCommand(address, PiRelayCommand.RelayOff, relay, 0, 0, ignoreChecks);
        }

        public static void RelayToggleAsync(byte address, byte relay, bool ignoreChecks = false)
        {
            SendCommand(address, PiRelayCommand.RelayToggle, relay, 0, 0, ignoreChecks);
        }

        public static void RelayToggleAsync(byte relay, bool ignoreChecks = false)
        {
            var r = GetAddressRelay(relay);
            SendCommand(r.Item1, PiRelayCommand.RelayToggle, r.Item2, 0, 0, ignoreChecks);
        }

        public static void RelayOnAsync(byte relay, bool ignoreChecks = false)
        {
            var r = GetAddressRelay(relay);
            SendCommand(r.Item1, PiRelayCommand.RelayOn, r.Item2, 0, 0, ignoreChecks);
        }

        public static void RelayOffAsync(byte relay, bool ignoreChecks = false)
        {
            var r = GetAddressRelay(relay);
            SendCommand(r.Item1, PiRelayCommand.RelayOff, r.Item2, 0, 0, ignoreChecks);
        }

        internal static Tuple<byte, byte> GetAddressRelay(byte relay)
        {
            var address = (byte) ((relay-1) / 7);
            var deviceRelay = (byte)(relay - (address * 7));
            return new Tuple<byte, byte>(address, deviceRelay);
        }
        
        public static void RelayAllAsync(byte address, byte relays, bool ignoreChecks = false)
        {
            if (relays > 127) throw new PiRelayException("Argument out of range. Must be between 0 and 127");

            SendCommand(address, PiRelayCommand.RelayAll, relays, 0, 0, ignoreChecks);
        }

        public static byte RelayStateAsync(byte address, bool ignoreChecks = false)
        {
            var r = SendCommand(address, PiRelayCommand.RelayState, 0, 0, 1, ignoreChecks);
            return r[0];
        }
    }
}
