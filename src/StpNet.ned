//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
package stp;

simple StpBridge
{
    parameters:
        @display("i=device/switch");
        int bridgeId;
        int priority = default(32768);
        double helloTime @unit(s) = default(2s);
        double maxAge @unit(s) = default(20s);
        bool rapid = default(false);
        double probeInterval @unit(s) = default(0s);
        double probeStart @unit(s) = default(8s);
        double probeEnd @unit(s) = default(23s);
    gates:
        inout ethg[];
}

network StpNet
{
    @display("bgb=292,338");
    submodules:
        b0: StpBridge {
            bridgeId = 0;
            @display("p=127,241");
        }
        b1: StpBridge {
            bridgeId = 1;
            @display("p=225,65");
        }
        b2: StpBridge {
            bridgeId = 2;
            @display("p=55,56");
        }
    connections:
        b0.ethg++ <--> StpLink <--> b1.ethg++;
        b1.ethg++ <--> StpLink <--> b2.ethg++;
        b2.ethg++ <--> StpLink <--> b0.ethg++;
}

channel StpLink extends ned.DelayChannel
{
    delay = default(10ms);
    int pathCost = default(1);
    double failAt @unit(s) = default(-1s);
    double restoreAt @unit(s) = default(-1s);
}

network StpFive
{
    parameters:
        bool diagonal = default(true);
        @display("bgb=550,330,#f6f8fc");
    submodules:
        b0: StpBridge {
            bridgeId = 0;
            @display("p=350,55");
        }
        b1: StpBridge {
            bridgeId = 1;
            @display("p=480,150");
        }
        b2: StpBridge {
            bridgeId = 2;
            @display("p=430,270");
        }
        b3: StpBridge {
            bridgeId = 3;
            @display("p=270,270");
        }
        b4: StpBridge {
            bridgeId = 4;
            @display("p=220,150");
        }
    connections:
        b0.ethg++ <--> link01: StpLink <--> b1.ethg++;
        b1.ethg++ <--> StpLink <--> b2.ethg++;
        b2.ethg++ <--> StpLink <--> b3.ethg++;
        b3.ethg++ <--> StpLink <--> b4.ethg++;
        b4.ethg++ <--> link04: StpLink <--> b0.ethg++;
        b1.ethg++ <--> StpLink <--> b4.ethg++ if diagonal;
}

network StpEight extends StpFive
{
    @display("bgb=550,330,#f6f8fc");
    submodules:
        b5: StpBridge {
            bridgeId = 5;
            @display("p=120,65");
        }
        b6: StpBridge {
            bridgeId = 6;
            @display("p=55,150");
        }
        b7: StpBridge {
            bridgeId = 7;
            @display("p=120,235");
        }
    connections:
        b4.ethg++ <--> StpLink <--> b5.ethg++;
        b5.ethg++ <--> StpLink <--> b6.ethg++;
        b6.ethg++ <--> StpLink <--> b7.ethg++;
        b7.ethg++ <--> StpLink <--> b4.ethg++;
}
