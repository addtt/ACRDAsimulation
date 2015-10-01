//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// The Aloha network consists of hosts which talk to the central "server" via
// the Aloha or Slotted Aloha protocol
//
network Acrda
{
    parameters:
        int numHosts;  // number of hosts
        int nRep;
        int nSlots;
        double tPkt @unit(s);
        string arrivalType;
        //double txRate @unit(bps);  // transmission rate
        @display("bgi=background/terrain,s");
    submodules:
        server: Server {
                numHosts = numHosts;
                nRep = nRep;
                nSlots = nSlots;
                tPkt = tPkt;
        }
        host[numHosts]: Host {
                nRep = nRep;
                nSlots = nSlots;
                tPkt = tPkt;
                arrivalType = arrivalType;
        }
}

