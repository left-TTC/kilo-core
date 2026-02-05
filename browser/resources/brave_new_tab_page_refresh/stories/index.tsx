/* Copyright (c) 2025 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import type { Meta, StoryObj } from '@storybook/react'

import '../../../../components/brave_new_tab_ui/stories/default/data/mockBraveNewsController'

import { NewTabContext } from '../context/new_tab_context'
import { BackgroundContext } from '../context/background_context'
import { SearchContext } from '../context/search_context'
import { TopSitesContext } from '../context/top_sites_context'
import { VpnContext } from '../context/vpn_context'
import { RewardsContext } from '../context/rewards_context'

import { createNewTabStore } from './mock_new_tab_store'
import { createBackgroundStore } from './mock_background_store'
import { createRewardsStore } from './mock_rewards_store'
import { createSearchStore } from './mock_search_store'
import { createTopSitesStore } from './mock_top_sites_store'
import { createVpnStore } from './mock_vpn_store'

import { StorybookArgs } from './storybook_args'

import { App } from '../components/app'

function StorybookApp(props: StorybookArgs) {
  return (
    <NewTabContext.Provider value={createNewTabStore()}>
      <BackgroundContext.Provider value={createBackgroundStore(props)}>
        <SearchContext.Provider value={createSearchStore()}>
          <TopSitesContext.Provider value={createTopSitesStore()}>
            <VpnContext.Provider value={createVpnStore()}>
              <RewardsContext.Provider value={createRewardsStore()}>
                <div style={{ position: 'absolute', inset: 0 }}>
                  <App />
                </div>
              </RewardsContext.Provider>
            </VpnContext.Provider>
          </TopSitesContext.Provider>
        </SearchContext.Provider>
      </BackgroundContext.Provider>
    </NewTabContext.Provider>
  )
}

export default {
  title: 'New Tab Page/App',
  component: StorybookApp,
} satisfies Meta<typeof StorybookApp>

export const NewTabPage: StoryObj<typeof StorybookApp> = {
  argTypes: {
    sponsoredBackgroundType: {
      control: 'select',
      options: ['none', 'image', 'rich'],
    },
  },
}
