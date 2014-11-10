/*
 * zenoss_events.h
 *
    ###########################################################################
    #
    # This program is part of Zenoss Core, an open source monitoring platform.
    # Copyright (C) 2008, Zenoss Inc.
    #
    # This program is free software; you can redistribute it and/or modify it
    # under the terms of the GNU General Public License version 2 as published by
    # the Free Software Foundation.
    #
    # For complete information please visit: http://www.zenoss.com/oss/
    #
    ###########################################################################
 *
 *  Created on: Aug 19, 2008
 *      Author: cgibbons
 */

#ifndef ZENOSS_EVENTS_H_
#define ZENOSS_EVENTS_H_

struct event_context *zenoss_event_context_init(TALLOC_CTX *mem_ctx, int (*callback)(int, uint16_t));

#endif /* ZENOSS_EVENTS_H_ */
