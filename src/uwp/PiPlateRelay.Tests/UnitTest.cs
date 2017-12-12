
using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace PiPlateRelay.Tests
{
    [TestClass]
    public class PiPlateRelayTests
    {
        [TestMethod]
        public void AddressPortFromByte()
        {
            var r = PiRelay.GetAddressRelay(7);
            Assert.IsTrue(r.Item1 == 0, "Incorrect Address");
            Assert.IsTrue(r.Item2 == 7, "Incorrect Port");
            r = PiRelay.GetAddressRelay(8);
            Assert.IsTrue(r.Item1 == 1, "Incorrect Address");
            Assert.IsTrue(r.Item2 == 1, "Incorrect Port");
            r = PiRelay.GetAddressRelay(1);
            Assert.IsTrue(r.Item1 == 0, "Incorrect Address");
            Assert.IsTrue(r.Item2 == 1, "Incorrect Port");
            r = PiRelay.GetAddressRelay(14);
            Assert.IsTrue(r.Item1 == 1, "Incorrect Address");
            Assert.IsTrue(r.Item2 == 7, "Incorrect Port");
            r = PiRelay.GetAddressRelay(15);
            Assert.IsTrue(r.Item1 == 2, "Incorrect Address");
            Assert.IsTrue(r.Item2 == 1, "Incorrect Port");
        }
    }
}
